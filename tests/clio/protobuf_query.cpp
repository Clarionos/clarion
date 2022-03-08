#include <boost/core/demangle.hpp>
#include <catch2/catch.hpp>
#include <iostream>

#include <clio/compress.hpp>
#include <clio/from_bin/varint.hpp>
#include <clio/from_json/varint.hpp>
#include <clio/json/any.hpp>
#include <clio/to_bin/varint.hpp>
#include <clio/to_json/varint.hpp>

#include <clio/bytes/from_json.hpp>
#include <clio/bytes/to_json.hpp>
#include <clio/to_json/map.hpp>
#include <clio/translator.hpp>
#include <fstream>

#include <chrono>
#include <clio/flatbuf.hpp>

struct service
{
   int add(int a, int b) const
   {
      std::cout << a << " + " << b << "\n";
      return a + b;
   }
   int sub(int a, int b)
   {
      std::cout << a << " - " << b << "\n";
      return a - b;
   }
};

CLIO_REFLECT(service, add, sub)

struct nest {
  uint8_t na;
  uint16_t nb;
  std::string nstr;
};
CLIO_REFLECT( nest, na, nb, nstr )

struct simple {
     uint8_t a;
     uint16_t b;
     uint32_t c;
     uint64_t d;
     std::string str = "hello";
     std::vector<nest> vn;
     std::variant<double,int> var;
     nest n;
};
CLIO_REFLECT( simple, a, b, c, d, str, vn, n, var )

TEST_CASE("flat schema", "[flat schema]")
{
   clio::schema service_schema;
   service_schema.generate<simple>();
   std::cout << "flat schema: " << format_json(service_schema) << "\n";


        auto decay_type = [&]( const std::string& str, bool& is_repeated ) -> std::string {
            if( str.back() == ']' ) { 
                is_repeated = true; 
                return str.substr( 0, str.size()-2); 
            }
            else if( str.back() == '?' ) return str.substr( 0, str.size()-1);
       //     else if( str.back() == '|' ) return convert_variant_name( str );
       //     else if( str.back() == '&' ) return convert_tuple_name( str );
       //     else if( str.back() == '>' ) return convert_map_name( str );
            else return str;
        };

  auto convert_to_js_type = [&]( const std::string& str, bool& is_repeated, bool& is_prim ) -> std::string {
      auto decay = decay_type( str, is_repeated );
      is_prim = true;
           if( decay == "char" ) return "Int8";
      else if( decay == "int8_t" ) return "Int8";
      else if( decay == "uint8_t" ) return "UInt8";
      else if( decay == "int16_t" ) return "Int16LE";
      else if( decay == "uint16_t" ) return "UInt16LE";
      else if( decay == "int32_t" ) return "Int32LE";
      else if( decay == "uint32_t" ) return "UInt32LE";
      else if( decay == "int64_t" ) return "Int64LE";
      else if( decay == "uint64_t" ) return "UInt64LE";
      else if( decay == "int8" ) return "Int8";
      else if( decay == "uint8" ) return "UInt8";
      else if( decay == "int16" ) return "Int16LE";
      else if( decay == "uint16" ) return "UInt16LE";
      else if( decay == "int32" ) return "Int32LE";
      else if( decay == "uint32" ) return "UInt32LE";
      else if( decay == "int64" ) return "Int64LE";
      else if( decay == "uint64" ) return "UInt64LE";
      else if( decay == "bool" ) return "UInt8";
      else if( decay == "double" ) return "DoubleLE";
      else if( decay == "float" ) return "FloatLE";
      is_prim = false;
      return decay;
  };


   for( const auto& item : service_schema.types ) {
       std::visit( [&]( auto i )  {
            if constexpr ( std::is_same_v<clio::object_type,decltype(i)> ) {
               std::cout << "class " << item.first<< " { \n";
               for( const auto& m : i.members ) {
                  bool is_prim; 
                  bool is_repeated;
                  bool is_array = m.type[m.type.size()-1] == ']';

                  auto name = convert_to_js_type(m.type,is_repeated,is_prim);
                  /// todo: handle dynamic sized types..

                  if( is_array ) {
                     std::cout << "    get "<<m.name<<"() { \n";
                     std::cout << "        offsetptr = bops.readUInt16LE( this.buffer, "<<m.offset<<" )\n";
                     std::cout << "        if( 0 == offsetptr ) return [];\n";
                     std::cout << "    }\n";
                  } else if( name == "string" ) {
                     std::cout << "    get "<<m.name<<"() { \n";
                     std::cout << "        offsetptr = bops.readUInt16LE( this.buffer, "<<m.offset<<" )\n";
                     std::cout << "        if( 0 == offsetptr ) return \"\";\n";
                     std::cout << "        offsetptr += " << m.offset + 4 <<";\n";
                     std::cout << "        size = bops.readUInt32LE( this.buffer, offsetptr )\n";
                     std::cout << "        strbuf = bops.subarray( this.buffer, offsetptr+4, size );\n";
                     std::cout << "        return bops.to( strbuf, encoding=\"utf8\" );\n";
                     std::cout << "    }\n";
                  } else if( is_prim ) {
                     std::cout << "    get "<<m.name<<"() { return bops.read" << name <<"( this.buffer, "<<m.offset<<" ) }\n";
                  } else {
                     std::cout << "    get "<<m.name<<"() { return new " << name
                                       <<"( bops.subarray( this.buffer, "<<m.offset<<", "<< m.size <<" ) )  }\n";
                  }
               }
               std::cout << "}\n";
            }
       }, item.second );
   }

}

TEST_CASE("try calling methods via protobuf", "[protobuf]")
{
   clio::schema service_schema;
   service_schema.generate<service>();
   service_schema.generate<clio::protobuf::query>();
   std::cout << "schema: " << format_json(service_schema) << "\n";

   clio::reflect<service>::proxy<clio::protobuf::query_proxy> query;
   query.add()(2, 3);
   query.add()(8, 5);
   query.sub()(2, 3);

   std::cout << clio::format_json(query->q) << "\n";
   auto protobuf_query = clio::to_protobuf(query->q);

   clio::input_stream protobuf_query_in(protobuf_query.data(), protobuf_query.size());
   std::string json_query;
   clio::string_stream json_query_out(json_query);

   clio::translate_protobuf_to_json(service_schema, "query", protobuf_query_in, json_query_out);

   std::cout << "pbuf query: " << format_json(clio::bytes{protobuf_query}) << "\n";
   std::cout << "json pbuf query: " << json_query << "\n";
   std::cout << "json query: "
             << clio::format_json(clio::protobuf::to_json_query<service>(query->q)) << "\n";

   service s;
   auto result = clio::protobuf::dispatch(s, query->q);

   std::cout << clio::format_json(result) << "\n";

   auto protobuf_result = clio::to_protobuf(result);
   std::string json_result;

   clio::input_stream protobuf_in(protobuf_result.data(), protobuf_result.size());
   clio::string_stream json_out(json_result);

   clio::translate_protobuf_to_json(service_schema, "service", protobuf_in, json_out);

   std::cout << "json result: " << json_result << "\n";
}
