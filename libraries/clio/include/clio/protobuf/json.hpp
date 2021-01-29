#pragma once
#include <clio/to_protobuf.hpp>
#include <clio/from_protobuf.hpp>
#include <clio/json/any.hpp>
#include <clio/protobuf/query.hpp>
#include <clio/to_json.hpp>

namespace clio {

    namespace protobuf {

        struct json_field_query;
        struct json_query {
            std::vector<json_field_query> fields;
        };
        struct json_field_query {
            std::string name;
            json::any   args;  /// args as json encoded tuple, or named object
            json_query  filter; /// apply to result 
        };
        CLIO_REFLECT( json_field_query, name, args, filter )
        CLIO_REFLECT( json_query, fields )


        /**
         * Given a json description of the query and an assumed reflected type (used to interpret names to numbers),
         * this function will convert the JSON-query into a protobuf-style query.
         */
        template<typename T>
        protobuf::query from_json_query( const json_query& jq ) {
            protobuf::query result;
            result.fields.reserve( jq.fields.size() );

            if constexpr( reflect<T>::is_struct ) {
                for( const auto& field : jq.fields ) {
                    reflect<T>::for_each( [&](const meta& ref, auto mptr ) {
                        if constexpr( not std::is_member_function_pointer_v< decltype(mptr) > ) {
                            if( ref.name   == field.name ) {
                                if( field.filter.fields.size() ) {
                                    result.fields.push_back({
                                        .number = ref.number, 
                                        .filter = from_json_query<std::decay_t<decltype(static_cast<T*>(nullptr)->*mptr)>>( field.filter ) 
                                    });
                                }
                                else {
                                    result.fields.push_back( field_query{ ref.number } );
                                }
                            }
                        } else { /// member function ptr
                            if( ref.name   == field.name ) {
                                /// TODO: do a direct conversion from json::any to pb bytes instead of
                                /// any->json_string->native->pb
                                using args_tuple_type = decltype( args_as_tuple( mptr ) );
                                std::string json = convert_to_json( field.args );
                                auto args_value = clio::from_json<args_tuple_type>(std::move(json));

                                if( field.filter.fields.size() ) {
                                    result.fields.push_back({
                                        .number = ref.number, 
                                        .args   = to_protobuf( args_value ),
                                        .filter = from_json_query< decltype( result_of(mptr) )>( field.filter ) 
                                    });
                                } else {
                                   result.fields.push_back( field_query{
                                        .number = ref.number, 
                                        .args   = to_protobuf( args_value )
                                   });
                                }
                            }
                        }
                    });
                }
            }
            return result;
        }

        /**
         *  Given a protobuf query and a type, convert it into a friendly JSON Query
         */
        template<typename T>
        json_query to_json_query( const protobuf::query& pbuf ) {
            json_query result;

            if constexpr( reflect<T>::is_struct ) {
                for( const auto& entry : pbuf.fields ) {
                    reflect<T>::for_each( [&](const meta& ref, auto mptr ) {
                        if constexpr( not std::is_member_function_pointer_v< decltype(mptr) > ) {
                            if( ref.number == entry.number.value ) {
                                if( entry.filter.fields.size() ) {
                                    result.fields.push_back({
                                        .name = ref.name, 
                                        .filter = to_json_query<std::decay_t<decltype(static_cast<T*>(nullptr)->*mptr)>>( entry.filter ) 
                                    });
                                }
                                else {
                                    result.fields.push_back( { ref.name} );
                                }
                            }
                        } else { /// member function ptr
                            if( ref.number == entry.number.value ) {
                                /// TODO: do a direct conversion from PB to json::any instead of
                                ///  pb->native->json_string->any
                                using args_tuple_type = decltype( args_as_tuple( mptr ) );
                                auto pbargs = from_protobuf<args_tuple_type>( entry.args.data );
                                auto jargs  = to_json( pbargs );

                                if( entry.filter.fields.size() ) {
                                    result.fields.push_back({
                                        .name   = ref.name, 
                                        .args   = from_json<json::any>( jargs ),
                                        .filter = to_json_query< decltype( result_of(mptr) )>( entry.filter ) 
                                    });
                                } else {
                                   result.fields.push_back( {
                                        .name  = ref.name, 
                                        .args   = from_json<json::any>( jargs )
                                   });
                                }
                            }
                        }
                    });
                }
            }
            return result;
        }

    } /// namespace protobuf
    template <typename S>
    void to_json(const protobuf::json_query& s, S& stream) {
       to_json(s.fields, stream);
    }
    template <typename S>
    void from_json( protobuf::json_query& s, S& stream) {
       from_json(s.fields, stream);
    }
    template <typename S>
    void to_json(const protobuf::json_field_query& s, S& stream) {
       stream.write('{');
         increase_indent(stream);
         write_newline(stream);
         stream.write("\"name\"");
         write_colon(stream);
         to_json( s.name, stream );
         if( auto* a = s.args.get_if<json::any_array>() ) {
             if( a->size() ) {
                 stream.write(',');
                 write_newline(stream);
                 stream.write("\"args\"");
                 write_colon(stream);
                 to_json( s.args, stream );
             }
         }
         else if( not s.args.get_if<json::null_t>() ) {
             stream.write(',');
             write_newline(stream);
             stream.write("\"args\"");
             write_colon(stream);
             to_json( s.args, stream );
         }
         if( s.filter.fields.size() ) {
             stream.write(',');
             write_newline(stream);
             stream.write("\"filter\"");
             write_colon(stream);
             to_json( s.filter, stream );
         }
         decrease_indent(stream);
         write_newline(stream);
       stream.write('}');
    }

} /// clio
