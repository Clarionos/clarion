#pragma once

#include <clio/error.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>
#include <string.h>
#include <algorithm>
#include <system_error>

namespace clio {
enum class stream_error {
   no_error,
   overrun,
   underrun,
   doubleread,
   float_error,
   varuint_too_big,
   invalid_varuint_encoding,
   bad_variant_index,
   invalid_asset_format,
   array_size_mismatch,
   invalid_name_char,
   invalid_name_char13,
   name_too_long,
   json_writer_error, // !!!
}; // stream_error
} // namespace clio 

namespace std {
template <>
struct is_error_code_enum<clio::stream_error> : true_type {};
} // namespace std

namespace clio {

class stream_error_category_type : public std::error_category {
 public:
   const char* name() const noexcept override final { return "ConversionError"; }

   std::string message(int c) const override final {
      switch (static_cast<stream_error>(c)) {
            // clang-format off
         case stream_error::no_error:                 return "No error";
         case stream_error::overrun:                  return "Stream overrun";
         case stream_error::underrun:                 return "Stream underrun";
         case stream_error::float_error:              return "Float error";
         case stream_error::varuint_too_big:          return "Varuint too big";
         case stream_error::invalid_varuint_encoding: return "Invalid varuint encoding";
         case stream_error::bad_variant_index:        return "Bad variant index";
         case stream_error::invalid_asset_format:     return "Invalid asset format";
         case stream_error::array_size_mismatch:      return "T[] size and unpacked size don't match";
         case stream_error::invalid_name_char:        return "character is not in allowed character set for names";
         case stream_error::invalid_name_char13:      return "thirteenth character in name cannot be a letter that comes after j";
         case stream_error::name_too_long:            return "string is too long to be a valid name";
         case stream_error::json_writer_error: return "Error writing json";
            // clang-format on

         default: return "unknown";
      }
   }
}; // stream_error_category_type

inline const stream_error_category_type& stream_error_category() {
   static stream_error_category_type c;
   return c;
}

inline std::error_code make_error_code(stream_error e) { return { static_cast<int>(e), stream_error_category() }; }


template<typename T>
constexpr bool has_bitwise_serialization() {
   if constexpr (std::is_arithmetic_v<T>) {
      return true;
   } else if constexpr (std::is_enum_v<T>) {
      static_assert(!std::is_convertible_v<T, std::underlying_type_t<T>>, "Serializing unscoped enum");
      return true;
   } else {
      return false;
   }
}

template <int max_size>
struct small_buffer {
   char  data[max_size];
   char* pos{ data };

   void reverse() { std::reverse(data, pos); }
};

struct string_stream {
   std::string& data;
   string_stream(std::string& data) : data(data) {}

   void write(char ch) {
      data += ch;
   }

   void write(const void* src, size_t size) {
      auto s = reinterpret_cast<const char*>(src);
      data.insert(data.end(), s, s + size);
   }

   template <int size>
   void write(const char (&src)[size]) {
      write(src, size-1);
   }

   template <typename T>
   void write_raw(const T& v) {
      write(&v, sizeof(v));
   }

   void write(const std::string& v) {
      write(v.c_str(), v.size());
   }
};


struct vector_stream {
   std::vector<char>& data;
   vector_stream(std::vector<char>& data) : data(data) {}

   void write(char ch) {
      data.push_back(ch);
   }

   void write(const void* src, size_t size) {
      auto s = reinterpret_cast<const char*>(src);
      data.insert(data.end(), s, s + size);
   }

   template <int size>
   void write(const char (&src)[size]) {
      write(src, size-1);
   }

   template <typename T>
   void write_raw(const T& v) {
      write(&v, sizeof(v));
   }

   void write(const std::string& v) {
      write(v.c_str(), v.size());
   }
};

struct fixed_buf_stream {
   char* begin;
   char* pos;
   char* end;

   fixed_buf_stream(char* pos, size_t size) : begin(pos), pos{ pos }, end{ pos + size } {}

   void write(char ch) {
      if (pos >= end)
         throw_error(stream_error::overrun);
      *pos++ = ch;
   }

   void write(const void* src, size_t size) {
      if (pos + size > end)
         throw_error(stream_error::overrun);
      memcpy(pos, src, size);
      pos += size;
   }

   template <int size>
   void write(const char (&src)[size]) {
      write(src, size-1);
   }

   template <typename T>
   void write_raw(const T& v) {
      write(&v, sizeof(v));
   }

   void write(const std::string& v) {
      write(v.c_str(), v.size());
   }

   void skip( int32_t s ) {
       if( (pos + s > end) or (pos + s < begin) )
           throw_error( stream_error::overrun );
       pos += s;
   }
   auto get_pos()const { return pos; }

   size_t remaining() { return end - pos; }
   size_t consumed()const { return pos-begin; }
};

struct size_stream {
   size_t size = 0;

   size_t get_pos()const { return size; }
   void write(char ch) {
      ++size;
   }
   size_t consumed()const { return size; }

   void write(const void* src, size_t size) {
      this->size += size;
   }

   template <int size>
   void write(const char (&src)[size]) {
      this->size += size-1;
   }

   template <typename T>
   void write_raw(const T& v) {
      size += sizeof(v);
   }

   void write(const std::string& v) {
      write(v.c_str(), v.size());
   }

   void skip( int32_t s ) {
       size += s;
   }
};

template <typename S>
void increase_indent(S&) { }

template <typename S>
void decrease_indent(S&) { }

template <typename S>
void write_colon(S& s) {
   s.write(':');
}

template <typename S>
void write_newline(S&) {
}

template <typename Base>
struct pretty_stream : Base {
   using Base::Base;
   int               indent_size = 4;
   std::vector<char> current_indent;
};

template <typename S>
void increase_indent(pretty_stream<S>& s) {
   s.current_indent.resize(s.current_indent.size() + s.indent_size, ' ');
}

template <typename S>
void decrease_indent(pretty_stream<S>& s) {
   if (s.current_indent.size() < s.indent_size)
      throw_error(stream_error::overrun);
   s.current_indent.resize(s.current_indent.size() - s.indent_size);
}

template <typename S>
void write_colon(pretty_stream<S>& s) {
   s.write(": ", 2);
}

template <typename S>
void write_newline(pretty_stream<S>& s) {
   s.write('\n');
   s.write(s.current_indent.data(), s.current_indent.size());
}

struct input_stream {
   const char* pos;
   const char* end;

   input_stream() : pos{ nullptr }, end{ nullptr } {}
   input_stream(const char* pos, size_t size) : pos{ pos }, end{ pos + size } {
      if ( size < 0 )
         throw_error( stream_error::overrun );
   }
   input_stream(const char* pos, const char* end) : pos{ pos }, end{ end } {
      if ( end < pos )
         throw_error( stream_error::overrun );
   }
   input_stream(const std::vector<char>& v) : pos{ v.data() }, end{ v.data() + v.size() } {}
   input_stream(std::string_view v) : pos{ v.data() }, end{ v.data() + v.size() } {}
   input_stream(const input_stream&) = default;

   input_stream& operator=(const input_stream&) = default;

   size_t remaining() { return end - pos; }

   void check_available(size_t size) {
      if (size > size_t(end - pos))
         throw_error(stream_error::overrun);
   }

   auto get_pos()const { return pos; }

   void checkread( size_t size) {
      if (size > size_t(end - pos))
         throw_error( stream_error::overrun );
      pos += size;
   }
   void read(void* dest, size_t size) {
      if (size > size_t(end - pos))
         throw_error( stream_error::overrun );
      memcpy(dest, pos, size);
      pos += size;
   }

   template <typename T>
   void read_raw(T& dest) {
      read(&dest, sizeof(dest));
   }

   void skip(size_t size) {
      if (size > size_t(end - pos))
         throw_error(stream_error::overrun);
      pos += size;
   }

   void read_reuse_storage(const char*& result, size_t size) {
      if (size > size_t(end - pos))
         throw_error( stream_error::overrun );
      result = pos;
      pos += size;
   }
};


struct check_input_stream {
   const char* begin;
   const char* pos;
   const char* end;
   size_t  total_read = 0;

   check_input_stream() : pos{ nullptr }, end{ nullptr } {}
   check_input_stream(const char* pos, size_t size) :begin(pos),pos{ pos },end{ pos + size } {
      if ( size < 0 )
         throw_error( stream_error::overrun );
   }
   check_input_stream(const char* pos, const char* end) : begin(pos),pos{ pos }, end{ end } {
      if ( end < pos )
         throw_error( stream_error::overrun );
   }
   check_input_stream(const std::vector<char>& v) : begin(v.data()), pos{ v.data() }, end{ v.data() + v.size() } {}
   check_input_stream(std::string_view v) : begin(v.data()),pos{ v.data() }, end{ v.data() + v.size() } {}
   check_input_stream(const check_input_stream&) = default;

   check_input_stream& operator=(const check_input_stream&) = default;

   size_t remaining() { return end - pos; }

   void check_available(size_t size) {
      if (size > size_t(end - pos))
         throw_error(stream_error::overrun);
   }

   auto get_pos()const { return pos; }

   void checkread( size_t size) {
      if (size > size_t(end - pos))
         throw_error( stream_error::overrun );
      pos += size;
      total_read += size;
   }
   void read(void* dest, size_t size) {
      if (size > size_t(end - pos))
         throw_error( stream_error::overrun );
      memcpy(dest, pos, size);
      pos += size;
      total_read += size;
   }

   template <typename T>
   void read_raw(T& dest) {
      read(&dest, sizeof(dest));
   }

   void skip(size_t size) {
      if (size > size_t(end - pos))
         throw_error(stream_error::overrun);
      pos += size;
      total_read += size;
   }

   void read_reuse_storage(const char*& result, size_t size) {
      if (size > size_t(end - pos))
         throw_error( stream_error::overrun );
      result = pos;
      pos += size;
      total_read += size;
   }


};







} // namespace clio
