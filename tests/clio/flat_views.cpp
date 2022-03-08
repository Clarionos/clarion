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
//#include <clio/safeflatbuf.hpp>

struct non_ref_nest
{
   float lite = 33;
};
struct non_reflect_type
{
   int first = 0;
   int second = 0;
   double real = 3.14;
   non_ref_nest nrn;
};

struct no_sub
{
   double myd = 3.14;
   uint16_t my16 = 22;
   ;
};
CLIO_REFLECT(no_sub, myd, my16);

struct sub_obj
{
   int a;
   int b;
   int c;
   std::string substr;
   no_sub ns;
};
CLIO_REFLECT(sub_obj, a, b, c, substr, ns);

struct flat_object
{
   non_reflect_type non_ref;
   uint32_t x;
   double y;
   std::string z;
   std::vector<int> veci;
   std::vector<std::string> vecstr = {"aaaaaaaaa", "bbbbbbbbbbb", "ccccccccccccc"};
   std::vector<no_sub> vecns;
   std::vector<flat_object> nested;
   sub_obj sub;
   clio::flat_ptr<sub_obj> sub_view;
};
CLIO_REFLECT(flat_object, non_ref, x, y, z, veci, vecstr, vecns, nested, sub, sub_view);

struct fat
{
   uint16_t a;
   uint16_t b;
   std::string substr;
};
CLIO_REFLECT(fat, a, b, substr);

struct root
{
   std::string other;
   clio::flat_ptr<fat> fatptr;
};
CLIO_REFLECT(root, other, fatptr);


TEST_CASE("flatptr")
{
   constexpr uint32_t offseta =
       clio::get_tuple_offset<0, std::tuple<sub_obj, clio::flat_ptr<sub_obj>>>::value;
   constexpr uint32_t offsetb =
       clio::get_tuple_offset<1, std::tuple<sub_obj, clio::flat_ptr<sub_obj>>>::value;
   std::cout << offseta << "  " << offsetb << "\n";
   std::cout << "flatpacksize: " << clio::flatpack_size<sub_obj>() << "\n";
   std::cout << "contains_offset_ptr: " << clio::contains_offset_ptr<sub_obj>() << "\n";

   /*
   clio::flat_ptr<root> r( root{ .other = "other", .fatptr = fat{ .a = 0x0A, .b = 0x0B, .substr =
   "sub" } } ); std::cout << "r.size: " << r.size() << "\n"; std::vector<char> d( r.data(),
   r.data()+r.size() ); std::cout << clio::to_json(clio::bytes{d}) <<"\n";

   std::cout << std::hex <<  r->fatptr->get()->a << "\n";
   std::cout << std::hex <<  r->fatptr->get()->b << "\n";
   std::cout << std::hex <<  r->fatptr->get()->substr << "\n";
   std::cout << std::dec <<  r->fatptr->get()->substr << "\n";
   */

   flat_object tester;
   tester.z = "my oh my";
   tester.x = 99;
   tester.y = 99.99;
   tester.veci = {1, 2, 3, 4, 6};
   tester.vecns = {{.myd = 33.33}, {}};
   tester.nested = {{.x = 11, .y = 3.21, .nested = {{.x = 88}}}, {.x = 33, .y = .123}};
   tester.sub.substr = "zzzzzzzzzz";
   tester.sub.ns.myd = .9876;
   tester.sub.ns.my16 = 33;
   tester.sub.a = 3;
   tester.sub.b = 4;
   tester.sub_view = sub_obj{.a = 0xdddd, .b = 0xbbbb, .substr = "yyyyyyyyyy"};

   clio::flat_ptr<flat_object> me(tester);
   std::cout << "fpsize:  " << clio::flatpack_size<flat_object>() << "\n";
   //    std::cerr<< "sub.a: " << std::hex << me->sub->a<<"\n";
   std::cerr << "sub_view->get()->a: \n";
   int x = me->sub_view()->a();
   std::cerr << "sub_view.a: " << std::hex << x << "\n";
   /*

   int y = me->sub->a.get();
   std::cerr<< "sub.a: " << std::hex << y <<"\n";

   std::vector<char> d( me.data(), me.data()+me.size() );
   std::cout << clio::to_json(clio::bytes{d}) <<"\n";
   std::cout << "start: " << std::dec << int64_t(me.data()) <<"\n";
   */
}
TEST_CASE("flatview")
{
   flat_object tester;
   tester.z = "my oh my";
   tester.x = 99;
   tester.y = 99.99;
   tester.veci = {1, 2, 3, 4, 6};
   tester.vecns = {{.myd = 33.33}, {}};
   tester.nested = {{.x = 11, .y = 3.21, .nested = {{.x = 88, .y=.321}}}, {.x = 33, .y = .123}};
   tester.sub.substr = "sub str";
   tester.sub.ns.myd = .9876;
   tester.sub.ns.my16 = 33;
   tester.sub.a = 3;
   tester.sub.b = 4;
   tester.sub_view = sub_obj{.a = 987, .b = 654, .substr = "subviewstr"};

   clio::flat_ptr<flat_object> me(tester);
   clio::flat_ptr<flat_object> sme(tester);
   std::cerr << "me.size: " << me.size() << "\n";
   std::cerr << "sme.size: " << sme.size() << "\n";

   auto start = std::chrono::steady_clock::now();
   uint64_t sum = 0;
   for (uint32_t i = 0; i < 10000; ++i)
   {
      for (uint32_t x = 0; x < me->veci()->size(); ++x)
         sum += me->veci()[x];
   }
   auto end = std::chrono::steady_clock::now();
   auto delta = end - start;

   auto view_time = (delta).count();
   std::cout << "sum veci via view:    " << std::chrono::duration<double, std::milli>(delta).count()
             << " ms\n";



   start = std::chrono::steady_clock::now();
   sum = 0;
   for (uint32_t i = 0; i < 10000; ++i)
   {
      for (uint32_t x = 0; x < sme->veci()->size(); ++x)
         sum += sme->veci()[x];
   }
   end = std::chrono::steady_clock::now();
   delta = end - start;

   view_time = (delta).count();
   std::cout << "sum veci via safe view:    " << std::chrono::duration<double, std::milli>(delta).count()
             << " ms\n\n";





   start = std::chrono::steady_clock::now();
   sum = 0;
   for (uint32_t i = 0; i < 10000; ++i)
   {
      for (uint32_t x = 0; x < tester.veci.size(); ++x)
         sum += tester.veci[x];
   }
   end = std::chrono::steady_clock::now();
   delta = end - start;
   std::cout << "sum: " << sum << "\n";
   std::cout << "sum veci via real: " << std::chrono::duration<double, std::milli>(delta).count()
             << " ms\n";
   auto real_time = (delta).count();

   std::cout << "view runtime: " << 100 * double(view_time) / real_time << "% of real\n";

   std::cout << "sub.substr: " << me->sub()->substr() << "\n";
   std::cout << "sub.substr: " << sme->sub()->substr() << "\n";

   //std::string d = sme->sub()->a;
   std::cout << "sub.substr: " << sme->sub()->a()<< "\n";


   /*
   std::cout << "x: " << me->x << "\n";
   std::cout << "x: " << sme->x() << "\n";
   std::cout << "y: " << me->y << "\n";
   std::cout << "y: " << sme->y() << "\n";
   std::cout << "z: " << me->z << "\n";
   std::cout << "z: " << sme->z() << "\n";
   std::cout << "veci[0]: " << me->veci[0] << "\n";
   std::cout << "veci[0]: " << sme->veci()[0] << "\n";
   std::cout << "veci[1]: " << me->veci[1] << "\n";
   std::cout << "veci[2]: " << me->veci[2] << "\n";
   std::cout << "veci[3]: " << me->veci[3] << "\n";
   std::cout << "veci[4]: " << me->veci[4] << "\n";
   std::cout << "vecstr[0]: " << me->vecstr[0] << "\n";
   std::cout << "vecstr[1]: " << me->vecstr[1] << "\n";
   std::cout << "vecstr[2]: " << me->vecstr[2] << "\n";
   std::cout << "vecns[0].myd: " << me->vecns[0].myd << "\n";
   std::cout << "vecns[0].my16: " << me->vecns[0].my16 << "\n";
   std::cout << "vecns[1].myd: " << me->vecns[1].myd << "\n";
   std::cout << "vecns[1].my16: " << me->vecns[1].my16 << "\n";
   std::cout << "nested->size(): " << me->nested->size() << "\n";
   std::cout << "nested[0].x: " << me->nested[0].x << "\n";
   std::cout << "nested[0].y: " << me->nested[0].y << "\n";
   std::cout << "nested[0].z: " << me->nested[0].z << "\n";
   std::cout << "nested[0].nested[0].x: " << me->nested[0].nested[0].x << "\n";
   std::cout << "nested[0].nested[0].y: " << me->nested[0].nested[0].y << "\n";
   std::cout << "safe nested[0].nested[0].y: " << sme->nested()[0].nested()[0].y() << "\n";
   std::cout << "nested[1].z: " << me->nested[1].z << "\n";
   std::cout << "nested[1].x: " << me->nested[1].x << "\n";
   std::cout << "nested[1].y: " << me->nested[1].y << "\n";
   std::cout << "sub.a: " << me->sub->a << "\n";
   std::cout << "sub.b: " << me->sub->b << "\n";
   std::cout << "sub.ns.myd: " << me->sub->ns->myd << "\n";
   std::cout << "sub.ns.my16: " << me->sub->ns->my16 << "\n";
   */
   std::cout << "nested[0].size " << me->nested()->size() <<"\n";
   std::cout << "nested[0].x: " << me->nested()[1].x() << "\n";
   std::cout << "nested[0].y: " << me->nested()[1].y() << "\n";
   std::cout << "nested[0].nested.size " << me->nested()[0].nested()->size() <<"\n";
   std::cout << "nested[0].nested[0].x: " << me->nested()[0].nested()[0].x() << "\n";
   std::cout << "nested[0].nested[0].y: " << me->nested()[0].nested()[0].y() << "\n";

   clio::input_stream in(me.data(), me.size());
   clio::flatcheck<flat_object>(in);

   std::cout << "sub_view.a: " << me->sub_view()->a() << "\n";
   //    std::cout << "sub_view.b: " << me->sub_view->b<<"\n";

   std::cout << "tester.sub.substr: " << tester.sub.substr << "\n";

   flat_object copy = me;  /// unpacks from buffer into full object
   /*
   std::cout << "copy.sub.a: " << copy.sub.a << "\n";
   std::cout << "copy.sub.b: " << copy.sub.b << "\n";
   std::cout << "copy.sub.ns.myd: " << copy.sub.ns.myd << "\n";
   std::cout << "copy.sub.ns.my16: " << copy.sub.ns.my16 << "\n";
   std::cout << "copy.sub.substr: " << copy.sub.substr << "\n";
   std::cout << "copy.sub_view->a: " << copy.sub_view->a << "\n";
   std::cout << "copy.sub_view->b: " << copy.sub_view->b << "\n";
   std::cout << "copy.sub_view->b: " << copy.sub_view->substr << "\n";

   std::cout << "non_ref->real: " << me->non_ref->real << "\n";
   std::cout << "non_ref->nrn.lite: " << me->non_ref->nrn.lite << "\n";
   */
}
