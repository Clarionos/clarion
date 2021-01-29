#pragma once
#include <clio/stream.hpp>

namespace clio {

template<typename InStream, typename OutStream>
void capp_unpack( InStream& in, OutStream& out ) {
    while( in.remaining() ) {
        uint8_t word_bits;
        in.read(&word_bits,1);

        const uint64_t zero_word = 0;

        if( word_bits == 0x00 ) {
            out.write(&zero_word,8);

            uint8_t zero_words;
            in.read( &zero_words, 1 );

            while( zero_words ) {
                out.write(&zero_word,8);
                --zero_words;
            }
            if( in.pos >= in.end ) {
               throw_error( clio::stream_error::overrun );
            }
        } else if( word_bits == 0xff ) {
            if( in.remaining() < 8 ) {
                out.write( in.pos, in.remaining() );
                in.skip( in.remaining() );
                return;
            }

            uint64_t word;
            in.read( &word, sizeof(word) );
            out.write( &word, 8 );

            uint8_t raw_words;
            in.read( &raw_words,1);
            if( raw_words ) {
                if( in.remaining() < uint32_t(raw_words * 8) ) 
                    throw_error( clio::stream_error::overrun );
                out.write( in.pos, uint32_t(raw_words) * 8 );
                in.pos += uint32_t(raw_words) * 8;
            }
        } else {
            uint64_t out_word = 0; 
            uint8_t* out_p = (uint8_t*)&out_word;
            for( auto i = 7; i >= 0; --i ) {
                if( word_bits & (1<<i) ) {
                    in.read(out_p,1);
                    out_p++;
                } else { 
                    out_p++;
                }
            }
            out.write( &out_word, sizeof(out_word) );
        }
    }
}

template<typename InStream, typename OutStream>
void capp_pack( InStream& in, OutStream& out ) {
    while( in.remaining() >= 8 ) {
        uint64_t next_word;
        in.read_raw(next_word);

        if( not next_word ) {
            out.write(char(0));

            uint32_t num_zero = 0;
            in.read_raw(next_word);

            while( not next_word && num_zero < 254) {
                in.read_raw(next_word);
                ++num_zero;
            }
            out.write( char(num_zero) );
        }


        uint8_t*   inbyte = (uint8_t*)&next_word;
        uint8_t*   endin  = inbyte + 8;

        uint8_t    temp[9];
        uint8_t&   zeros = temp[0];
        uint8_t*   outbyte = &temp[1];
        zeros = 0;

        do {
            zeros <<= 1;
            if( bool(*inbyte) ) {
                zeros += 1;
                *outbyte = *inbyte;
                ++outbyte;
            }
            ++inbyte;
        } while( inbyte != endin );
        out.write( temp, outbyte-temp );

        if( zeros == 0xff ) {
            uint8_t num_raw = 0;
            auto* p = in.pos;
            auto* e = in.end;
            while( *p and p != e ) ++p;

            num_raw =  (p - in.pos)/8;
            out.write( num_raw );
            out.write( in.pos, num_raw * 8 );
            in.skip( num_raw * 8 );

          //  in.read_raw(next_word);
            /// TODO: once we enter "raw" mode we shouldn't exit until 
            /// we have seen a certain percentage of zero bytes... for example,
            /// one zero per word isn't worth encoding... 
        }


    }
    if( in.remaining() ) {
        out.write( char(0xff) ); /// indicate all bytes are present, receiver will truncate
        out.write( in.pos, in.remaining() );
        in.skip( in.remaining() );
    }
    return;
};

inline std::vector<char> capn_compress( const std::vector<char>& c ) {
    clio::input_stream in( c.data(), c.size() );
    /*
    clio::size_stream sizess;
    capp_unpack( in, sizess);
    in.pos = c.data();
    */

    std::vector<char>  out;
    out.reserve( c.size() );
    clio::vector_stream vout(out);
    capp_pack( in, vout );
    return out;
}
inline std::vector<char> capn_uncompress( const std::vector<char>& c ) {
    clio::input_stream in( c.data(), c.size() );
    /*
    clio::size_stream sizess;
    capp_unpack( in, sizess);
    in.pos = c.data();
    */

    std::vector<char>  out;
    out.reserve( c.size()*1.5 );
    clio::vector_stream vout(out);
    capp_unpack( in, vout );
    return out;
}

} /// clio
