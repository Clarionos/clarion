#pragma once
#include <clio/from_bin.hpp>
#include <clio/tuple.hpp>

namespace clio {

template<typename T, typename S>
void from_protobuf_object( T& obj, S& stream );

template <int N = 0, typename... T>
bool set_variant(std::variant<T...>& result, uint32_t type) {
   if (type == N) {
      result.template emplace<N>();
      return true;
   } else if constexpr (N + 1 < sizeof...(T)) {
      return set_variant<N + 1>(result, type);
   }
   return false;
}

template<typename S>
void skip_member( uint16_t wire_type, S& stream ) {
    uint32_t temp = 0;
    switch( wire_type ) {
        case 0: 
            varuint32_from_bin( temp, stream ); 
            break;
        case 1: 
            stream.skip(8); 
            break; 
        case 2: 
            varuint32_from_bin( temp, stream );
            stream.skip(temp);
            break;
        case 5: 
            stream.skip(4);
            break; 
    }
}


template <typename... Ts, typename S>
void from_protobuf_object(std::variant<Ts...>& obj, S& stream)
{
    uint32_t key = 0;
    varuint32_from_bin( key, stream );
    uint32_t field = key >> 3;

    if( set_variant( obj, field-1 ) ) {
        std::visit([&](auto& x) { 
           return from_protobuf_member(x, stream); 
        }, obj);
    } else {
        skip_member( uint8_t(key) & 0x07 , stream );
        obj = std::variant<Ts...>(); /// reset to default
    }
}
template <typename... Ts, typename S>
void from_protobuf_object(std::tuple<Ts...>& obj, S& stream)
{
    while( stream.remaining() ) {
        uint32_t key = 0;
        varuint32_from_bin( key, stream );
        uint16_t wire_type = uint8_t(key) & 0x07;
        uint32_t number    = key >> 3;

        bool skip_it = true;
        tuple_get( obj, number-1, [&]( auto& member ) {
             from_protobuf_member( member, stream );
             skip_it = false;
        });
        if( skip_it ) skip_member( wire_type, stream );
    }
}


template<typename T, typename S>
void from_protobuf_object( std::vector<T>& obj, S& stream ) {
    obj.clear();    
    if constexpr( std::is_arithmetic_v<T> ) {
        uint32_t key = 0;
        varuint32_from_bin( key, stream );
        uint16_t wire_type = uint8_t(key) & 0x07;
        uint32_t number    = key >> 3;
        if( number != 1 )
            skip_member( wire_type, stream );
        else {
            uint32_t size = 0;
            varuint32_from_bin( size, stream );
            if( size > stream.remaining() ) 
                throw_error(stream_error::overrun);
            obj.resize( size / sizeof( T ) );
            stream.read( (char*)obj.data(), size );
        }
    } else {
        while( stream.remaining() ) {
            uint32_t key = 0;
            varuint32_from_bin( key, stream );
            uint16_t wire_type = uint8_t(key) & 0x07;
            uint32_t number    = key >> 3;

            bool skip_it = true;
            if( number == 1 ) {
                skip_it = false;
                obj.resize(obj.size()+1);
                from_protobuf_member( obj.back(), stream );
            } else {
                skip_member( wire_type, stream );
            }
        }
    }
}

template <typename T, typename S>
void from_protobuf_member( T& obj, S& stream ) {
    if constexpr( std::is_same_v< T, std::string> ) {
        uint32_t size = 0;
        varuint32_from_bin( size, stream );
        obj.resize(size);
        stream.read( obj.data(), size );
    } else if constexpr( std::is_same_v< T, bytes> ) {
        uint32_t size = 0;
        varuint32_from_bin( size, stream );
        obj.data.resize(size);
        stream.read( obj.data.data(), size );
    } 
    else if constexpr( reflect<T>::is_struct || is_std_variant<T>::value || is_std_vector<T>::value || is_std_tuple<T>::value ) {
        uint32_t size = 0;
        varuint32_from_bin( size, stream );
        if( size > stream.remaining() ) 
            throw_error(stream_error::overrun);
        input_stream objstream( stream.pos, size );
        stream.skip(size);
        from_protobuf_object( obj, objstream );
    } else if constexpr ( 5 == wire_type<T>::value or 1 == wire_type<T>::value  ) {
        from_bin( obj, stream );
    } else if constexpr ( 0 == wire_type<T>::value )  {
        uint32_t v;
        varuint32_from_bin( v, stream );
        obj = v;
    } else {
        T::from_protobuf_is_not_defined; /// used to generate useful compile error
    }
}

template<typename T, typename S>
void from_protobuf_object( T& obj, S& stream ) {
    while( stream.remaining() ) {
        uint32_t key = 0;
        varuint32_from_bin( key, stream );
        uint16_t wire_type = uint8_t(key) & 0x07;
        uint32_t number    = key >> 3;

        //bool skip_it = true;
        if( not reflect<T>::get( number, [&]( auto m ) {
            if constexpr ( std::is_member_function_pointer_v<decltype(m)> ) {
                skip_member( wire_type, stream ); /// we cannot store return value of functions in T
            } else {
                //using member_type = std::decay_t<decltype( obj.*m )>;
                //auto& member = obj.*m;
                from_protobuf_member( obj.*m, stream );
            }
        }) ) { // if not reflect<T>::get(number...)
            skip_member( wire_type, stream );
        }
    }
}

template<typename T, typename S>
T from_protobuf( S& stream ) {
    T tmp;
    from_protobuf_object( tmp, stream );
    return tmp;
}

template<typename T>
T from_protobuf( const std::vector<char>& in ) {
    clio::input_stream stream(in.data(), in.size() );
    return clio::from_protobuf<T>(stream);
}
template<typename T>
T from_protobuf( std::vector<char>& in ) {
    clio::input_stream stream(in.data(), in.size() );
    return clio::from_protobuf<T>(stream);
}

} // namespace clio
