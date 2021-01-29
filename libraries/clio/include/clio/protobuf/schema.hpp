#pragma once
#include <clio/schema.hpp>

namespace clio {

    /**
     *  Writes a protobuf schem file to stream given a schema object
     */
    template<typename S>
    void to_protobuf_schema( const schema& sch, S& stream ) {

        stream.write( "syntax = \"proto3\";\n" );

        auto convert_variant_name = []( string name ) {
            for( auto& c : name ) {
                if( c == '|' ) c = '_';
                if( c == '?' ) c = 'O';
                if( c == '[' ) c = '_';
                if( c == ']' ) c = 'a';
            }
            return "variant_" + name.substr( 0, name.size()-1);
        };
        auto convert_tuple_name = []( string name ) {
            for( auto& c : name ) {
                if( c == '&' ) c = '_';
                if( c == '?' ) c = 'O';
                if( c == '[' ) c = '_';
                if( c == ']' ) c = 'a';
            }
            return "tuple_" + name.substr( 0, name.size()-1);
        };

        auto convert_map_name = []( string name ) {
            /* //TODO...
            for( auto& c : name ) {
                if( c == '&' ) c = '_';
                if( c == '?' ) c = 'O';
                if( c == '[' ) c = '_';
                if( c == ']' ) c = 'a';
            }
            */
            return name;
        };

        auto decay_type = [&]( const std::string& str, bool& is_repeated ) -> std::string {
            if( str.back() == ']' ) { 
                is_repeated = true; 
                return str.substr( 0, str.size()-2); 
            }
            else if( str.back() == '?' ) return str.substr( 0, str.size()-1);
            else if( str.back() == '|' ) return convert_variant_name( str );
            else if( str.back() == '&' ) return convert_tuple_name( str );
            else if( str.back() == '>' ) return convert_map_name( str );
            else return str;
        };

        auto convert_to_pb_type = [&]( const string& str, bool& is_repeated, bool& is_prim ) -> std::string {
            auto decay = decay_type( str, is_repeated );
            is_prim = true;
            if( decay == "int16_t" ) return "int32";
            else if( decay == "char" ) return "int32";
            else if( decay == "int8_t" ) return "int32";
            else if( decay == "uint16_t" ) return "int32";
            else if( decay == "uint8_t" ) return "int32";
            else if( decay == "int32_t" ) return "sfixed32";
            else if( decay == "uint32_t" ) return "fixed32";
            else if( decay == "int64_t" ) return "sfixed64";
            else if( decay == "uint64_t" ) return "fixed64";
            else if( decay == "bool" ) return "bool";
            else if( decay == "double" ) return decay;
            else if( decay == "float" ) return decay;
            is_prim = false;
            return decay;
        };

        for( const auto& item : sch.types ) {
            std::visit( [&]( auto i )  {
               if constexpr ( std::is_same_v<object_type,decltype(i)> ) {
                    stream.write( "message ", 8 );
                    stream.write( item.first );
                    stream.write( " {\n" );
                    for( auto mem : i.members ) {
                       bool is_repeated = false;
                       bool is_prim = false;
                       auto pbt = convert_to_pb_type( mem.type, is_repeated, is_prim );
                       stream.write( "    " );
                       if( is_repeated ) {
                          stream.write( "repeated " );
                       }
                       stream.write( pbt );
                       stream.write( " " );
                       stream.write( mem.name.c_str() );
                       stream.write( " = " );
                       stream.write( std::to_string( mem.number ).c_str()  );
                        if( is_repeated && is_prim ) {
                            stream.write( " [packed=true]" );
                        }
                       stream.write( ";\n" );
                    }
                    /*
                    for( auto mem : i.methods ) {
                       bool is_repeated = false;
                       bool is_prim = false;
                       auto pbt = convert_to_pb_type( mem.return_type, is_repeated, is_prim );
                       stream.write( "    " );
                       stream.write( "repeated " );
                       stream.write( pbt );
                       stream.write( " " );
                       stream.write( mem.name.c_str() );
                       stream.write( " = " );
                       stream.write( std::to_string( mem.number ).c_str()  );
                       stream.write( ";\n" );
                    }*/
                    stream.write( "}\n" );
               } else if constexpr ( std::is_same_v<variant_type,decltype(i)> ) {
                    stream.write( "message " );
                    if( item.first.back() == '|' ) {
                        stream.write( convert_variant_name(item.first) );
                    } else {
                        stream.write( item.first );
                    }

                    stream.write( " {\n" );
                    stream.write( "   oneof which {\n" );

                    for( uint32_t idx = 0; idx < i.types.size(); ++idx ) {
                        bool is_repeated = false;
                        bool is_prim = false;
                        auto pbt = convert_to_pb_type( i.types[idx], is_repeated, is_prim  );

                        stream.write( "       " );
                        stream.write( pbt );
                        stream.write( "  " );
                        stream.write( pbt );
                        stream.write( "_value = " );
                        stream.write( std::to_string(idx+1));
                        stream.write( ";\n" );
                    }
                    stream.write( "   }\n" );
                    stream.write( "}\n" );
               } else if constexpr ( std::is_same_v<tuple_type,decltype(i)> ) {
                    stream.write( "message " );

                    if( item.first.back() == '&' ) {
                        stream.write( convert_tuple_name(item.first) );
                    } else {
                        stream.write( item.first );
                    }

                    stream.write( " {\n" );
                    for( uint32_t idx = 0; idx < i.types.size(); ++idx ) {
                        stream.write( "   " );
                        bool is_repeated = false;
                        bool is_prim = false;
                        auto pbt = convert_to_pb_type( i.types[idx], is_repeated, is_prim );
                        if( is_repeated ) {
                          stream.write( "repeated " );
                        }
                        stream.write( pbt );
                        stream.write( " _" );
                        stream.write( std::to_string(idx+1) );
                        stream.write( " = " );
                        stream.write( std::to_string(idx+1));
                        if( is_repeated && is_prim ) {
                            stream.write( " [packed=true]" );
                        }
                        stream.write( ";\n" );
                    }
                    stream.write( "}\n" );
               }
            }, item.second );
        }
    }
    inline std::string to_protobuf_schema( const schema& s ) {
       size_stream ss;
       to_protobuf_schema(s, ss);

       std::string      result(ss.size, 0);
       fixed_buf_stream fbs(result.data(), result.size());
       to_protobuf_schema(s, fbs);

       if (fbs.pos != fbs.end)
          throw_error(stream_error::underrun);

       return result;
    }
} /// clio
