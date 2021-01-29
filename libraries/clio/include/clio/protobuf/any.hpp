#pragma once
#include <variant>
#include <clio/to_bin/varint.hpp>
#include <clio/from_bin/varint.hpp>

namespace clio {
    namespace protobuf {
        enum wire_type_enum {
            varint  = 0,
            fixed64 = 1,
            buffer  = 2,
            fixed32 = 5
        };

        struct entry {
            typedef std::variant<varuint32,int64_t,bytes,int32_t> value_type;

            entry(){}
            template<typename T>
            entry( uint32_t n, T&& v )
            :number(n),value( std::forward<T>(v) ){}

            uint32_t    number;
            value_type  value;
        };
        CLIO_REFLECT( entry, number, value )

        /**
         *  This class can be used to hold the
         *  deserialized contents of any protobuf stream
         */
        struct any {
            std::vector< entry > members;

            void add( uint32_t field, const std::string& s ) {
                members.push_back( entry{ field, bytes{ std::vector<char>(s.begin(), s.end()) } } );
            }
            template<typename T, typename = std::enable_if< std::is_arithmetic<T>::value > >
            void add( uint32_t field, const std::vector<T>& s ) {
                members.push_back( entry{ field, 
                    bytes{ std::vector<char>( (char*)s.data(), (char*)(s.data()+s.size())) } } );
            }
            void add( uint32_t field, std::vector<char>&& s ) {
                members.push_back( entry{ field, bytes{ std::move(s) } } );
            }
            void add( uint32_t field, varuint32 s ) {
                members.push_back( entry{ field, s } );
            }
            void add( uint32_t field, uint64_t s ) {
                members.push_back( entry{ field, static_cast<int64_t>(s) } );
            }
            void add( uint32_t field, int64_t s ) {
                members.push_back( entry{ field, s } );
            }
            void add( uint32_t field, int32_t s ) {
                members.push_back( entry{ field, s } );
            }
            void add( uint32_t field, int16_t s ) {
                members.push_back( entry{ field, varuint32(s) } );
            }
            void add( uint32_t field, uint16_t s ) {
                members.push_back( entry{ field, varuint32(s) } );
            }
            void add( uint32_t field, int8_t s ) {
                members.push_back( entry{ field, varuint32(s) } );
            }
            void add( uint32_t field, uint8_t s ) {
                members.push_back( entry{ field, varuint32(s) } );
            }
            void add( uint32_t field, bool s ) {
                members.push_back( entry{ field, varuint32(s) } );
            }
            void add( uint32_t field, char s ) {
                members.push_back( entry{ field, varuint32(s) } );
            }
            void add( uint32_t field, double s ) {
                members.push_back( entry{ field, *reinterpret_cast<int64_t*>(&s) } );
            }
            void add( uint32_t field, float s ) {
                members.push_back( entry{ field, *reinterpret_cast<int32_t*>(&s) } );
            }
        };
        CLIO_REFLECT( any, members )


        template<typename Stream>
        void to_bin( const any& a, Stream& s ) {
            for( const auto& e : a.members ) {
                uint32_t key = e.number << 3;
                key |= e.value.index() == 3 ? wire_type_enum::fixed32 : e.value.index();
                varuint32_to_bin( key, s ); 

                std::visit( [&]( const auto& v ){
                   to_bin( v, s );
                }, e.value );
            }
        }

        template<typename Stream>
        void from_bin( any& a, Stream& s ) {
            while( s.remaining() ) {
               uint32_t key = 0;
               varuint32_from_bin( key, s );
               wire_type_enum type = wire_type_enum(uint8_t(key) & 0x07);
               uint32_t number = key >> 3;

               switch( type ) {
                   case wire_type_enum::varint: {
                      varuint32 val;
                      from_bin( val, s );
                      a.members.push_back( {number, val} );
                   } break;
                   case wire_type_enum::fixed64: {
                      int64_t val;
                      from_bin( val, s );
                      a.members.push_back( {number, val} );
                   } break;
                   case wire_type_enum::buffer: {
                      bytes val;
                      from_bin( val, s );
                      a.members.emplace_back( number, std::move(val) );
                   } break;
                   case wire_type_enum::fixed32: {
                      int32_t val;
                      from_bin( val, s );
                      a.members.push_back( {number, val} );
                   } break;
               }
            }
        }

    } /// namespace protobuf

    template<typename Stream>
    void to_protobuf( const protobuf::any& a, Stream& s ) {
        to_bin( a, s );
    }
    template<typename Stream>
    void from_protobuf( protobuf::any& a, Stream& s ) {
        from_bin( a, s );
    }

} /// namespace clio
