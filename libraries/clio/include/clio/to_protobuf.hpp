#pragma once
#include <clio/to_bin.hpp>
#include <clio/tuple.hpp>
#include <clio/bytes.hpp>

namespace clio {

template <typename... Ts, typename S>
void to_protobuf_object(const std::tuple<Ts...>& obj, S& stream);

template <typename... Ts, typename S>
void to_protobuf_object(const std::variant<Ts...>& obj, S& stream);

template <typename T, typename S>
void to_protobuf_object(const std::vector<T>& obj, S& stream);

template <typename T, typename S>
void to_protobuf_object(const T& obj, S& stream);

template<typename T>
struct wire_type { static constexpr const auto value = 2; };

#define WIRE_TYPE(X,T) \
template<> struct wire_type<X> { static constexpr const auto value = T; };


template<typename... T> struct wire_type< std::variant<T...>> { static constexpr const auto value = 2; };

WIRE_TYPE( uint16_t, 0 )
WIRE_TYPE( uint8_t, 0 )
WIRE_TYPE( int16_t, 0 )
WIRE_TYPE( int8_t, 0 )
WIRE_TYPE( char, 0 )
WIRE_TYPE( bool, 0 )
WIRE_TYPE( varuint32, 0 )
WIRE_TYPE( uint64_t, 1 )
WIRE_TYPE( int64_t, 1 )
WIRE_TYPE( double, 1 )
WIRE_TYPE( uint32_t, 5 )
WIRE_TYPE( int32_t, 5 )
WIRE_TYPE( float, 5 )
WIRE_TYPE( std::string, 2 )
WIRE_TYPE( bytes, 2 )

/**
 *  Assumes key has been writen, then writes the rest...
 */
template <typename T, typename S>
void to_protobuf_member( const T& obj, S& stream ) {
    if constexpr( std::is_same_v< T, std::string> ) {
        varuint32_to_bin( obj.size(), stream );
        stream.write( obj.data(), obj.size() );
    } else if constexpr( std::is_same_v< T, bytes> ) {
        varuint32_to_bin( obj.data.size(), stream );
        stream.write( obj.data.data(), obj.data.size());
    } else if constexpr ( 5 == wire_type<T>::value or 1 == wire_type<T>::value  ) {
        to_bin( obj, stream );
    } else if constexpr ( 0 == wire_type<T>::value )  {
        varuint32_to_bin( obj, stream );
    } else if constexpr( reflect<T>::is_struct || 
                       is_std_vector<T>::value || 
                       is_std_tuple<T>::value || 
                       is_std_variant<T>::value  ) 
    {
        size_stream ss;
        to_protobuf_object(obj, ss);
        varuint32_to_bin( ss.size, stream );
        to_protobuf_object(obj, stream);
    } else {
        T::to_protobuf_is_not_defined; /// used to generate useful compile error
    }
}

template<typename Member, typename Stream>
void write_protobuf_field( int field, const Member& member, Stream& stream ) {
    if constexpr( is_std_vector< std::decay_t<Member> >::value ) {
        if( member.size() == 0 ) return;
    }
    uint32_t key = (field << 3) | wire_type<Member>::value; 
    varuint32_to_bin( key, stream );
    to_protobuf_member( member, stream );
}

template <typename... Ts, typename S>
void to_protobuf_object(const std::variant<Ts...>& obj, S& stream) {
    std::visit( [&](auto m){ 
        write_protobuf_field( obj.index()+1, m, stream );
    }, obj );
}


/**
 *  A vector is protobuf object that is either packed in a single field or
 *  listed as N fields of the same type.
 */
template <typename T, typename S>
void to_protobuf_object(const std::vector<T>& vec, S& stream) {
    uint32_t key = (1 << 3) | wire_type<std::vector<T>>::value; 
    if constexpr ( std::is_arithmetic_v< T > ) { /// [packed=true]
        varuint32_to_bin( key, stream );
        auto size = uint32_t(vec.size() * sizeof( T ));
        varuint32_to_bin( size, stream );
        stream.write( vec.data(), vec.size()*sizeof(T) ); 
    }
    else {
        for( const auto& item : vec ) {
            varuint32_to_bin( key, stream );
            to_protobuf_member( item, stream );
        }
    }
}


template <typename T, typename S>
void to_protobuf_object(const T& obj, S& stream) {
  reflect<T>::for_each( [&]( const clio::meta& ref, auto m ) {
    if constexpr( not std::is_member_function_pointer_v<decltype(m)> ) {
        write_protobuf_field( ref.number, obj.*m, stream );
    }
  });
}

template <typename... Ts, typename S>
void to_protobuf_object(const std::tuple<Ts...>& obj, S& stream) {
    tuple_for_each( obj, [&]( int idx, const auto& m ) {
        write_protobuf_field( idx+1, m, stream );
    });
}

template <typename T, typename S>
void to_protobuf(const T& obj, S& stream) {
   to_protobuf_object( obj, stream );
}

template <typename T>
std::vector<char> to_protobuf(const T& t) {
   size_stream ss;
   to_protobuf(t, ss);

   std::vector<char>      result(ss.size, 0);
   fixed_buf_stream fbs(result.data(), result.size());

   to_protobuf(t, fbs);

   if (fbs.pos != fbs.end)
       throw_error( stream_error::underrun );
   return result;
}



} // clio
