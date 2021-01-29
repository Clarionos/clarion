#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <tuple>
#include <vector>

#include <clio/murmur.hpp>

namespace clio {

#define CLIO_REFLECT_TYPENAME( T ) \
   constexpr const char* get_type_name( const T* ) { return BOOST_PP_STRINGIZE(T); } 

#define CLIO_REFLECT_TYPENAME_CUSTOM( T, CUSTOM ) \
   constexpr const char* get_type_name( const T* ) { return BOOST_PP_STRINGIZE(CUSTOM); } 

constexpr const char* get_type_name(const bool*) { return "bool"; }
constexpr const char* get_type_name(const int8_t*) { return "int8"; }
constexpr const char* get_type_name(const uint8_t*) { return "uint8"; }
constexpr const char* get_type_name(const int16_t*) { return "int16"; }
constexpr const char* get_type_name(const uint16_t*) { return "uint16"; }
constexpr const char* get_type_name(const int32_t*) { return "int32"; }
constexpr const char* get_type_name(const uint32_t*) { return "uint32"; }
constexpr const char* get_type_name(const int64_t*) { return "int64"; }
constexpr const char* get_type_name(const uint64_t*) { return "uint64"; }
constexpr const char* get_type_name(const float*) { return "float32"; }
constexpr const char* get_type_name(const double*) { return "double"; }
constexpr const char* get_type_name(const char*) { return "char"; }
constexpr const char* get_type_name(const std::string*) { return "string"; }
constexpr const char* get_type_name(const __int128*) { return "int128"; }
constexpr const char* get_type_name(const unsigned __int128*) { return "uint128"; }

#ifdef __eosio_cdt__
constexpr const char* get_type_name(const long double*) { return "float128"; }
#endif

template <std::size_t N, std::size_t M>
constexpr std::array<char, N + M> array_cat(std::array<char, N> lhs, std::array<char, M> rhs) {
   std::array<char, N + M> result{};
   for (int i = 0; i < N; ++i) { result[i] = lhs[i]; }
   for (int i = 0; i < M; ++i) { result[i + N] = rhs[i]; }
   return result;
}

template <std::size_t N>
constexpr std::array<char, N> to_array(std::string_view s) {
   std::array<char, N> result{};
   for (int i = 0; i < N; ++i) { result[i] = s[i]; }
   return result;
}

template <typename T, std::size_t N>
constexpr auto append_type_name(const char (&suffix)[N]) {
   constexpr std::string_view name = get_type_name((T*)nullptr);
   return array_cat(to_array<name.size()>(name), to_array<N>({ suffix, N }));
}

template <typename T>
constexpr auto vector_type_name = append_type_name<T>("[]");

template <typename T>
constexpr auto optional_type_name = append_type_name<T>("?");

template <typename T>
constexpr const char* get_type_name(const std::vector<T>*) {
   return vector_type_name<T>.data();
}

template <typename T>
constexpr const char* get_type_name(const std::optional<T>*) {
   return optional_type_name<T>.data();
}

struct variant_type_appender {
   char*                           buf;
   constexpr variant_type_appender operator+(std::string_view s) {
      *buf++ = '_';
      for (auto ch : s) *buf++ = ch;
      return *this;
   }
};

template <typename... T>
constexpr auto get_variant_type_name() {
   constexpr std::size_t  size = sizeof("variant") + ((std::string_view(get_type_name((T*)nullptr)).size() + 1) + ...);
   std::array<char, size> buffer{ 'v', 'a', 'r', 'i', 'a', 'n', 't' };
   (variant_type_appender{ buffer.data() + 7 } + ... + std::string_view(get_type_name((T*)nullptr)));
   buffer[buffer.size() - 1] = '\0';
   return buffer;
}

template <typename... T>
constexpr auto get_tuple_type_name() {
   constexpr std::size_t  size = sizeof("tuple") + ((std::string_view(get_type_name((T*)nullptr)).size() + 1) + ...);
   std::array<char, size> buffer{ 't', 'u', 'p', 'l', 'e'  };
   (variant_type_appender{ buffer.data() + 5 } + ... + std::string_view(get_type_name((T*)nullptr)));
   buffer[buffer.size() - 1] = '\0';
   return buffer;
}

template <typename... T>
constexpr auto variant_type_name = get_variant_type_name<T...>();

template <typename... T>
constexpr const char* get_type_name(const std::variant<T...>*) {
   return variant_type_name<T...>.data();
}

template<typename T>
constexpr const char* get_type_name() {
   return get_type_name( (const T*)nullptr );
}

[[nodiscard]] inline constexpr bool char_to_name_digit_strict(char c, uint64_t& result) {
   if (c >= 'a' && c <= 'z') {
      result = (c - 'a') + 6;
      return true;
   }
   if (c >= '1' && c <= '5') {
      result = (c - '1') + 1;
      return true;
   }
   if (c == '.') {
      result = 0;
      return true;
   }
   return false;
}

[[nodiscard]] inline constexpr bool string_to_name_strict(uint64_t& name, std::string_view str) {
   name       = 0;
   unsigned i = 0;
   for (; i < str.size() && i < 12; ++i) {
      uint64_t x = 0;
      // - this is not safe in const expression OUTCOME_TRY(char_to_name_digit_strict(str[i], x));
      auto r = char_to_name_digit_strict(str[i], x);
      if( !r ) return false;
      name |= (x & 0x1f) << (64 - 5 * (i + 1));
   }
   if (i < str.size() && i == 12) {
      uint64_t x = 0;
      // - this is not safe in const expression OUTCOME_TRY(char_to_name_digit_strict(str[i], x));
      auto r = char_to_name_digit_strict(str[i], x);
      if( !r ) return false;

      if (x != (x & 0xf))
         return false;
      name |= x;
      ++i;
   }
   if (i < str.size())
      return false;
   return true;
}

inline constexpr uint64_t hash_name( std::string_view str ) {
    uint64_t r = 0;
    if( not string_to_name_strict(r, str) )
        return  murmur64( str.data(), str.size() );
    return r;
}

template<typename T>
constexpr uint64_t  get_type_hashname() {
   return hash_name( get_type_name<T>() );
}

} // namespace clio
