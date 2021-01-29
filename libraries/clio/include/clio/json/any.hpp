#pragma once
#include <clio/reflect.hpp>
#include <clio/to_json.hpp>
#include <clio/from_json.hpp>

namespace clio {

    namespace json {
        struct any;
        struct entry;
        struct null_t{};// uint8_t none = 0; };
        struct error_t{ std::string what; };

        using any_object = std::vector<entry>;
        using any_array  = std::vector<any>;

        struct any {
            public:
              any(){};
              template<typename T>
              any( const T& v ):_value(v){} 

              template<typename T>
              const T* get_if()const { return std::get_if<T>(&_value); }

              template<typename T>
              const T& as()const { 
                 if( auto p = std::get_if<T>(&_value) ) 
                    return *p;

                 assert( !"invalid type cast from any" );
                // static T dummy; /// used to prevent warnings about no return
                // return dummy;
              }
              template<typename T>
              T& as(){ 
                 if( auto p = std::get_if<T>(&_value) ) 
                    return *p;

                 assert( !"invalid type cast from any" );
                 __builtin_unreachable();
              }

              const auto& value()const { return _value; }
              auto&       value(){ return _value; }

              inline any operator[]( const std::string_view& key )const;
              any operator[]( uint32_t index )const {
                 if( auto p = get_if<any_array>() ) {
                    if( index >= p->size() ) return error_t{ "index out of bounds" };
                    return p->at(index);
                 }
                 return error_t{ "not an array" };
              }

              template<typename Lambda>
              void visit( Lambda&& l )const {
                  std::visit( std::forward<Lambda>(l), _value );
              }
            private:
                std::variant<null_t, 
                    std::string, 
                    int32_t, 
                    int64_t, 
                    double, 
                    bool, 
                    any_object, 
                    any_array,
                    error_t > _value;

              template <typename S>
              friend void to_bin(const any& obj, S& stream) {
                 to_bin( obj._value, stream );
              }
              template <typename S>
              friend void from_bin( any& obj, S& stream) {
                 from_bin( obj._value, stream );
              }
        };

        struct entry {
            std::string key;
            any    value;
        };
        CLIO_REFLECT( entry, key, value )
        CLIO_REFLECT_TYPENAME( null_t )
        CLIO_REFLECT( error_t, what )
        
        
        inline any any::operator[]( const std::string_view& key )const {
           if( auto p = get_if<any_object>() ) {
              for( const auto& i : *p ) {
                 if( i.key == key ) return i.value;
              }
           }
           return error_t{"not an object"};
        }

        template <typename S>
        void to_json(const any& obj, S& stream) {
           std::visit( [&]( const auto& val ) {
               using T = std::decay_t<decltype(val)>;
               if constexpr ( std::is_same_v<null_t,T> ) {
                  stream.write( "null", 4 ); 
               }
               else if constexpr ( std::is_same_v<any_object,T> ) {
                  stream.write( '{' );
                  int size = val.size();
                  for( const auto& e : val ) {
                    if( size == val.size() ) {
                       increase_indent(stream);
                    }
                    write_newline(stream);
                    to_json(e.key, stream);
                    write_colon(stream);
                    to_json(e.value, stream);
                    if( --size ) {
                       stream.write( ',' );
                    }
                  }
                  if( !val.empty() ) {
                     decrease_indent(stream);
                     write_newline(stream);
                  }
                  stream.write( '}' );
               } else {
                  to_json( val, stream );
               }
           }, obj.value() );
        }

        template <typename S>
        void from_json(any& obj, S& stream);

        template <typename S>
        void from_json(any_object& obj, S& stream) {
           stream.get_start_object();
           while( true ) {
              auto t = stream.peek_token();
              if (t.get().type == json_token_type::type_end_object)
                 break;
              auto k = stream.get_key();
              obj.push_back( { .key = std::string(k) } );
              from_json( obj.back().value, stream );
           }
           return stream.get_end_object();
        }

        template <typename S>
        void from_json(any& obj, S& stream) {
           while( true ) {
               auto t = stream.peek_token();
               switch( t.get().type ) {
                   case json_token_type::type_null: 
                       stream.eat_token();
                       obj = null_t{}; 
                       return;
                   case json_token_type::type_bool: 
                       obj = t.get().value_bool; 
                       stream.eat_token();
                       return;
                   case json_token_type::type_string: 
                       obj = std::string(t.get().value_string); 
                       stream.eat_token();
                       return;
                   case json_token_type::type_start_object: {
                       obj = any_object();
                       from_json( obj.as<any_object>(), stream );
                       return;
                   }
                   case json_token_type::type_start_array: {
                      obj = any_array();
                      from_json( obj.as<any_array>(), stream );
                      return;
                   }
                   case json_token_type::type_end_array:
                   case json_token_type::type_end_object:
                   case json_token_type::type_key:
                   default:
                      stream.eat_token();
                      throw_error(clio::from_json_error::value_invalid);
               };
           }
        }

    } /// namespace json

} /// clio
