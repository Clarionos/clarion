#pragma once
#include <map>
#include <clio/to_json.hpp>

namespace clio {
template <typename K, typename T, typename S>
void to_json( const std::map<K,T>& m, S& stream) {
   stream.write('{');
   if( m.size() == 0 ) {
       return stream.write('}');
   }
   increase_indent(stream);
   bool not_first = false;
   for( const auto& p : m ) {
       if( not_first )  {
           stream.write(',');
       }
       write_newline(stream);
       to_json( p.first, stream );
       write_colon(stream);
       to_json( p.second, stream );
       not_first = true;
   }
   decrease_indent(stream);
   write_newline(stream);
   return stream.write('}');
}
} /// namespace clio
