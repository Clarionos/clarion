#pragma once
#include <string>
#include <clio/get_type_name.hpp>
#include <clio/reflect.hpp>

namespace clio {

inline std::string name_to_string(uint64_t name) {
   static const char* charmap = ".12345abcdefghijklmnopqrstuvwxyz";
   std::string        str(13, '.');

   uint64_t tmp = name;
   for (uint32_t i = 0; i <= 12; ++i) {
      char c      = charmap[tmp & (i == 0 ? 0x0f : 0x1f)];
      str[12 - i] = c;
      tmp >>= (i == 0 ? 4 : 5);
   }

   const auto last = str.find_last_not_of('.');
   return str.substr(0, last + 1);
}

inline constexpr uint64_t char_to_name_digit(char c) {
   if (c >= 'a' && c <= 'z')
      return (c - 'a') + 6;
   if (c >= '1' && c <= '5')
      return (c - '1') + 1;
   return 0;
}

inline constexpr uint64_t string_to_name(const char* str, int size) {
   uint64_t name = 0;
   int      i    = 0;
   for (; i < size && i < 12; ++i) name |= (char_to_name_digit(str[i]) & 0x1f) << (64 - 5 * (i + 1));
   if (i < size)
      name |= char_to_name_digit(str[i]) & 0x0F;
   return name;
}

inline constexpr uint64_t string_to_name(const char* str) {
   int len = 0;
   while (str[len]) ++len;
   return string_to_name(str, len);
}

inline constexpr uint64_t string_to_name(std::string_view str) { return string_to_name(str.data(), str.size()); }

struct name {
   enum class raw : uint64_t {};
   uint64_t value = 0;

   constexpr name() = default;
   constexpr explicit name(uint64_t value) : value{ value } {}
   constexpr explicit name(name::raw value) : value{ static_cast<uint64_t>(value) } {}
   constexpr explicit name(std::string_view str) {
       if( not string_to_name_strict( value, str ) ) {
       // TODO:    throw_error
       }
   }


   constexpr name(const name&) = default;

   constexpr   operator raw() const { return static_cast<raw>(value); }
   explicit    operator std::string() const { return name_to_string(value); }
   std::string to_string() const { return std::string(*this); }
   /**
    * Explicit cast to bool of the uint64_t value of the name
    *
    * @return Returns true if the name is set to the default value of 0 else true.
    */
   constexpr explicit operator bool() const { return value != 0; }

   /**
    *  Converts a %name Base32 symbol into its corresponding value
    *
    *  @param c - Character to be converted
    *  @return constexpr char - Converted value
    */
   static constexpr uint8_t char_to_value(char c) {
      if (c == '.')
         return 0;
      else if (c >= '1' && c <= '5')
         return (c - '1') + 1;
      else if (c >= 'a' && c <= 'z')
         return (c - 'a') + 6;
      return 0; // control flow will never reach here; just added to suppress warning
   }

   /**
    *  Returns the length of the %name
    */
   constexpr uint8_t length() const {
      constexpr uint64_t mask = 0xF800000000000000ull;

      if (value == 0)
         return 0;

      uint8_t l = 0;
      uint8_t i = 0;
      for (auto v = value; i < 13; ++i, v <<= 5) {
         if ((v & mask) > 0) {
            l = i;
         }
      }

      return l + 1;
   }

   /**
    *  Returns the suffix of the %name
    */
   constexpr name suffix() const {
      uint32_t remaining_bits_after_last_actual_dot = 0;
      uint32_t tmp                                  = 0;
      for (int32_t remaining_bits = 59; remaining_bits >= 4;
           remaining_bits -= 5) { // Note: remaining_bits must remain signed integer
         // Get characters one-by-one in name in order from left to right (not including the 13th character)
         auto c = (value >> remaining_bits) & 0x1Full;
         if (!c) { // if this character is a dot
            tmp = static_cast<uint32_t>(remaining_bits);
         } else { // if this character is not a dot
            remaining_bits_after_last_actual_dot = tmp;
         }
      }

      uint64_t thirteenth_character = value & 0x0Full;
      if (thirteenth_character) { // if 13th character is not a dot
         remaining_bits_after_last_actual_dot = tmp;
      }

      if (remaining_bits_after_last_actual_dot ==
          0) // there is no actual dot in the %name other than potentially leading dots
         return name{ value };

      // At this point remaining_bits_after_last_actual_dot has to be within the range of 4 to 59 (and restricted to
      // increments of 5).

      // Mask for remaining bits corresponding to characters after last actual dot, except for 4 least significant bits
      // (corresponds to 13th character).
      uint64_t mask  = (1ull << remaining_bits_after_last_actual_dot) - 16;
      uint32_t shift = 64 - remaining_bits_after_last_actual_dot;

      return name{ ((value & mask) << shift) + (thirteenth_character << (shift - 1)) };
   }

   /**
    *  Returns the prefix of the %name
    */
   constexpr name prefix() const {
      uint64_t result                 = value;
      bool     not_dot_character_seen = false;
      uint64_t mask                   = 0xFull;

      // Get characters one-by-one in name in order from right to left
      for (int32_t offset = 0; offset <= 59;) {
         auto c = (value >> offset) & mask;

         if (!c) {                        // if this character is a dot
            if (not_dot_character_seen) { // we found the rightmost dot character
               result = (value >> offset) << offset;
               break;
            }
         } else {
            not_dot_character_seen = true;
         }

         if (offset == 0) {
            offset += 4;
            mask = 0x1Full;
         } else {
            offset += 5;
         }
      }

      return name{ result };
   }
};
//CLIO_REFLECT( name, value )


template <typename S>
void from_json(name& obj, S& stream) {
   auto r = stream.get_string();
   obj = name(hash_name(r)); 
}

template <typename S>
void to_json(const name& obj, S& stream) {
   to_json(name_to_string(obj.value), stream);
}

inline namespace literals {
   inline constexpr name operator""_n(const char* s, size_t) { return name( std::string_view(s) ); }
   inline constexpr name operator""_h(const char* s, size_t) { return name( hash_name(s) ); }
} // namespace literals

} // namespace clio
