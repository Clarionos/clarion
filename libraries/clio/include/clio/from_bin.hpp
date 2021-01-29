#pragma once
#include <deque>
#include <clio/bytes.hpp>
#include <clio/stream.hpp>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <tuple>
#include <variant>
#include <vector>
#include <string_view>

namespace clio {

template <typename T, typename S>
void from_bin(T& obj, S& stream);

template <typename S>
void varuint32_from_bin(uint32_t& dest, S& stream) {
   dest          = 0;
   int     shift = 0;
   uint8_t b     = 0;
   do {
      if (shift >= 35)
         throw_error(stream_error::invalid_varuint_encoding);
      from_bin(b, stream);
      dest |= uint32_t(b & 0x7f) << shift;
      shift += 7;
   } while (b & 0x80);
}

template <typename S>
void varuint64_from_bin(uint64_t& dest, S& stream) {
   dest          = 0;
   int     shift = 0;
   uint8_t b     = 0;
   do {
      if (shift >= 70)
         throw_error(stream_error::invalid_varuint_encoding);
      from_bin(b, stream);
      dest |= uint64_t(b & 0x7f) << shift;
      shift += 7;
   } while (b & 0x80);
}

template <typename S>
void varint32_from_bin(int32_t& result, S& stream) {
   uint32_t v;
   varuint32_from_bin(v, stream);
   if (v & 1)
      result = ((~v) >> 1) | 0x8000'0000;
   else
      result = v >> 1;
}

template <typename T, typename S>
void from_bin_assoc(T& v, S& stream) {
   uint32_t size;
   varuint32_from_bin(size, stream);
   for (size_t i = 0; i < size; ++i) {
      typename T::value_type elem;
      from_bin(elem, stream);
      v.emplace(elem);
   }
}

template <typename T, typename S>
void from_bin_sequence(T& v, S& stream) {
   uint32_t size;
   varuint32_from_bin(size, stream);
   for (size_t i = 0; i < size; ++i) {
      v.emplace_back();
      from_bin(v.back(), stream);
   }
}

template <typename T, std::size_t N, typename S>
void from_bin(T (&v)[N], S& stream) {
   uint32_t size;
   varuint32_from_bin(size, stream);
   if (size != N)
      throw_error(stream_error::array_size_mismatch);
   if constexpr (has_bitwise_serialization<T>()) {
      stream.read(reinterpret_cast<char*>(v), size * sizeof(T));
   } else {
      for (size_t i = 0; i < size; ++i) { OUTCOME_TRY(from_bin(v[i], stream)); }
   }
}

template <typename T, typename S>
void from_bin(std::vector<T>& v, S& stream) {
   if constexpr (has_bitwise_serialization<T>()) {
      if constexpr (sizeof(size_t) >= 8) {
         uint64_t size;
         varuint64_from_bin(size, stream);
         stream.check_available(size * sizeof(T));
         v.resize(size);
         stream.read(reinterpret_cast<char*>(v.data()), size * sizeof(T));
      } else {
         uint32_t size;
         varuint32_from_bin(size, stream);
         stream.check_available(size * sizeof(T));
         v.resize(size);
         stream.read(reinterpret_cast<char*>(v.data()), size * sizeof(T));
      }
   } else {
      uint32_t size;
      varuint32_from_bin(size, stream);
      v.resize(size);
      for (size_t i = 0; i < size; ++i) {
         from_bin(v[i], stream);
      }
   }
}

template <typename T, typename S>
void from_bin(std::set<T>& v, S& stream) {
   from_bin_assoc(v, stream);
}

template <typename T, typename U, typename S>
void from_bin(std::map<T, U>& v, S& stream) {
   uint32_t size;
   varuint32_from_bin(size, stream);
   for (size_t i = 0; i < size; ++i) {
      std::pair<T, U> elem;
      from_bin(elem, stream);
      v.emplace(elem);
   }
}

template <typename T, typename S>
void from_bin(std::deque<T>& v, S& stream) {
   from_bin_sequence(v, stream);
}

template <typename T, typename S>
void from_bin(std::list<T>& v, S& stream) {
   from_bin_sequence(v, stream);
}

template <typename S>
void from_bin(input_stream& obj, S& stream) {
   if constexpr (sizeof(size_t) >= 8) {
      uint64_t size;
      varuint64_from_bin(size, stream);
      stream.check_available(size);
      stream.read_reuse_storage(obj.pos, size);
      obj.end = obj.pos + size;
   } else {
      uint32_t size;
      varuint32_from_bin(size, stream);
      stream.check_available(size);
      stream.read_reuse_storage(obj.pos, size);
      obj.end = obj.pos + size;
   }
}

template <typename First, typename Second, typename S>
void from_bin(std::pair<First, Second>& obj, S& stream) {
   from_bin(obj.first, stream);
   from_bin(obj.second, stream);
}

template <typename S>
inline void from_bin(std::string& obj, S& stream) {
   uint32_t size;
   varuint32_from_bin(size, stream);
   obj.resize(size);
   stream.read(obj.data(), obj.size());
}

template <typename S>
inline void from_bin(std::string_view& obj, S& stream) {
   uint32_t size;
   varuint32_from_bin(size, stream);
   obj = std::string_view(stream.get_pos(),size);
   stream.skip(size);
}

template <typename T, typename S>
void from_bin(std::optional<T>& obj, S& stream) {
   bool present;
   from_bin(present, stream);
   if (!present) {
      obj.reset();
      return;
   }
   obj.emplace();
   from_bin(*obj, stream);
}

template <uint32_t I, typename... Ts, typename S>
void variant_from_bin(std::variant<Ts...>& v, uint32_t i, S& stream) {
   if constexpr (I < std::variant_size_v<std::variant<Ts...>>) {
      if (i == I) {
         auto& x = v.template emplace<I>();
         from_bin(x, stream);
      } else {
         variant_from_bin<I + 1>(v, i, stream);
      }
   } else {
      throw_error(stream_error::bad_variant_index);
   }
}

template <typename... Ts, typename S>
void from_bin(std::variant<Ts...>& obj, S& stream) {
   uint32_t u;
   varuint32_from_bin(u, stream);
   variant_from_bin<0>(obj, u, stream);
}

template <typename T, std::size_t N, typename S>
void from_bin(std::array<T, N>& obj, S& stream) {
   for (T& elem : obj) { from_bin(elem, stream); }
}

template <int N, typename T, typename S>
void from_bin_tuple(T& obj, S& stream) {
   if constexpr (N < std::tuple_size_v<T>) {
      from_bin(std::get<N>(obj), stream);
      from_bin_tuple<N + 1>(obj, stream);
   }}

template <typename... T, typename S>
void from_bin(std::tuple<T...>& obj, S& stream) {
   from_bin_tuple<0>(obj, stream);
}

template <typename T, typename S>
void from_bin(T& obj, S& stream) {
   if constexpr (has_bitwise_serialization<T>()) {
      stream.read(reinterpret_cast<char*>(&obj), sizeof(T));
   } else { //if constexpr (std::is_same_v<serialization_type<T>, void>) {
      reflect<T>::for_each( [&]( const clio::meta&, auto m ) {
        if constexpr( not std::is_member_function_pointer_v<decltype(m)> ) {
            from_bin( obj.*m, stream );
        }
      });
   } /*else {
      // TODO: This can operate in place for standard serializers
      decltype(serialize_as(obj)) temp;
      OUTCOME_TRY(from_bin(temp, stream));
      convert(temp, obj, choose_first);
      return outcome::success();
   } */
}

template <typename T>
void from_bin_vec(T& obj, const std::vector<char>& bin) {
   input_stream stream{ bin };
   from_bin(obj, stream);
}
template <typename T>
void from_bin_vec(T& obj, std::vector<char>& bin) {
   input_stream stream{ bin };
   from_bin(obj, stream);
}

template <typename T>
void from_bin_vec(T& obj, const bytes& bin) {
   from_bin_vec(obj, bin.data);
}
template <typename T>
void from_bin_vec(T& obj, bytes& bin) {
   from_bin_vec(obj, bin.data);
}

template <typename T>
T from_bin(const std::vector<char>& bin) {
   T    obj;
   from_bin_vec(obj, bin);
   return obj;
}

template <typename T, typename S>
T from_bin(S& stream) {
   T    obj;
   from_bin(obj, stream);
   return obj;
}




} // namespace clio
