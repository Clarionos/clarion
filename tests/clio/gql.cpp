#include <catch2/catch.hpp>
#include <clio/flatbuf.hpp>
#include <clio/gql.hpp>
#include <clio/name.hpp>
#include <clio/schema.hpp>
#include <clio/to_json.hpp>
#include <clio/to_json/map.hpp>
#include <iostream>

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

CLIO_REFLECT_PB(service, (add, 0, a, b), (sub, 1, a, b))

TEST_CASE("try flatpack/unpack/view of std::varaint", "[variant]")
{
   using variant_type = std::variant<int, double, std::string>;

   auto test_type = [](auto val) {
      variant_type test_variant;
      test_variant = val;
      clio::flat_ptr<variant_type> packed = test_variant;

      variant_type unpacked = packed;
      std::cout << "test_variant: " << clio::to_json(test_variant) << "\n";
      std::cout << "unpacked_variant: " << clio::to_json(unpacked) << "\n";

      packed->visit([](const auto& v) {
         std::cout << "visit value: " << v << "\n";
         /*
         if constexpr ( std::is_same_v<clio::flat<std::string>,std::decay_t<decltype(v)>> ) {
             std::cout << "json value: " <<  clio::format_json( std::string_view(v) )  <<"\n";
         } else {
             std::cout << "json value: " <<  clio::format_json(v)  <<"\n";
         }
         */
      });
      //        clio::input_stream in( packed.data(), packed.size() );
      //       clio::flatcheck<variant_type>( in );
   };
   test_type(33);
   test_type(33.33);
   test_type("hello world");
}

TEST_CASE("try flat pack/view of gql::query", "[gql]")
{
   clio::gql::query q;

   clio::schema service_schema;
   service_schema.generate<clio::gql::query>();
   //    std::cout << "schema: " << clio::format_json( service_schema )<<"\n";

   clio::flat_ptr<clio::gql::query> fq(q);
}
