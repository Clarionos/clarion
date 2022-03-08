#include <boost/core/demangle.hpp>
#include <catch2/catch.hpp>
#include <iostream>

#include <chrono>
#include <clio/fracpack.hpp>
#include <clio/to_bin.hpp>
#include "trans_generated.h"
#include "transfer_generated.h"

/**  
 
  # Types of Data

  ## 1 - Fixed Size / Fixed Structure (FF)
     - primitives and structs composed of primptives
     - no size prefix because there is no heap

         
  ## 2 - Dynamic Size / Fixed Structure  (DF)

     - strings, vectors, variants, and structs that contain them 
     - 32 bit size prefix which accounts for the fixed size and heap data

  ## 3 -  Dynamic Size / Extensible Structure   (DE)

     - the fixed size portion can change as well as the dynamic size
     - 32 bit size prefix which accounts for the fixed size and heap data
     - 16 bit fixed size prefix which accounts for the start of the heap
*/


/**
 *  This is the simplest possible case, the structure should be
 *  packed as if by memcpy. This requires that the struct is reflected
 *  in the same order.
 */
struct FRACPACK simple final {
   uint32_t a;
   uint64_t b;
   uint16_t c;
};
CLIO_REFLECT( simple, a, b, c )

struct simple_with_string final {
   uint32_t a;
   uint64_t b;
   uint16_t c;
   std::string s;
};
CLIO_REFLECT( simple_with_string, a, b, c, s )

struct ext_simple_with_string {
   uint32_t a;
   uint64_t b;
   uint16_t c;
   std::string s;
};
CLIO_REFLECT( ext_simple_with_string, a, b, c, s )

struct not_final_simple {
   uint32_t a;
   uint64_t b;
   uint16_t c;
};
CLIO_REFLECT( not_final_simple, a, b, c )

/** this struct requires alignement of something other than 1*/
struct req_align_simple final {
   uint64_t a;
   uint32_t b;
   uint16_t c;
   uint16_t d;
};
CLIO_REFLECT( req_align_simple, a, b, c, d )

struct FRACPACK mixed_simple final {
   uint32_t a;
   uint64_t b;
   uint16_t c;
};
CLIO_REFLECT( mixed_simple, c, a, b )

struct simple_with_mult_string final {
   uint32_t a;
   uint64_t b;
   uint16_t c;
   std::string s;
   std::string s2;
};
CLIO_REFLECT( simple_with_mult_string, a, b, c, s, s2 )

struct nested_final_struct final {
   uint32_t                a;
   simple_with_mult_string nest;
};
CLIO_REFLECT( nested_final_struct, a, nest )

struct nested_not_final_struct final {
   uint32_t                          a;
   not_final_simple                  nest;
};
CLIO_REFLECT( nested_not_final_struct, a, nest )

struct varstr final {
   std::variant<simple,int32_t, double> v;
};
CLIO_REFLECT( varstr, v )


struct FRACPACK test_view {
   uint32_t size; 
   uint16_t heap; // 4
   uint32_t offset_v; // 4
   uint8_t  type; // 0
   uint32_t a;
   uint64_t b;
   uint16_t c;
};

namespace clio {
template<typename T, typename P>
constexpr bool is_frac() {
   return std::is_same_v<frac<T>*, P>;
}
template<typename T, typename P>
constexpr bool is_const_frac() {
   return std::is_same_v<const frac<T>*, P>;
}
}

TEST_CASE("variant" ) {
   varstr vs{ .v=double(42.1234) }; //mixed_simple{.a=1, .b=2, .c=3} };


   REQUIRE( clio::fracpack_size(vs.v) == 9 ); 
   vs.v = simple{.a=1, .b=2, .c=3};
   REQUIRE( clio::fracpack_size(vs.v) == clio::fracpack_size(simple())+1 );

   clio::frac_ptr<varstr> p( { .v=simple{.a=111, .b=222, .c=333} } );
   REQUIRE( p.size() == 1+2+4+clio::fracpack_size(simple())  ); 

   auto tv = reinterpret_cast<test_view*>(p.data());
   std::cout << "tv->heap: "<<tv->heap<<"\n";
   std::cout << "tv->offset_v: "<<tv->offset_v<<"\n";
   std::cout << "tv->a: "<<tv->a<<"\n";
   std::cout << "tv->b: "<<tv->b<<"\n";
   std::cout << "tv->c: "<<tv->c<<"\n";

  
   
   p->v()->visit( [&]( auto v ) {
       if constexpr( clio::is_const_frac<int32_t,decltype(v)>() ) {
          std::cout<< (int32_t)*v;
       } else if constexpr( clio::is_const_frac<double,decltype(v)>() ) {
          std::cout<< (double)*v;
       } else {
         // std::cout <<"offset: " << (char*)&v->a()->val - p.data()+4 <<"\n";
          std::cout <<"a.val: "<< (uint32_t)v->a() <<"\n";
          std::cout <<"b.val: "<< (uint64_t)v->b() <<"\n";
          std::cout <<"c.val: "<< (uint16_t)v->c() <<"\n";
          std::cout <<"hello world!\n";
       }
   } );

   auto u = p.unpack();
}
 
TEST_CASE("fracpack-not-final-nest") {

   REQUIRE( clio::can_memcpy<not_final_simple>() == false );
   REQUIRE( clio::may_use_heap<std::optional<uint64_t>>() == true );
   REQUIRE( clio::may_use_heap<not_final_simple>() == true );
   REQUIRE( clio::may_use_heap<nested_not_final_struct>() == true );
   //REQUIRE( clio::can_memcpy_test<nested_not_final_struct>() == false );
   REQUIRE( clio::can_memcpy<nested_not_final_struct>() == false );

   
   nested_not_final_struct nfs{ .a = 123, .nest = { .a = 456, .b=789, .c=321 } };
   clio::frac_ptr<nested_not_final_struct> ptr(nfs);

   REQUIRE( (int)ptr->a() == 123 );
   REQUIRE( (int)ptr->nest()->a() == 456 );
   REQUIRE( (int)ptr->nest()->b() == 789);
   REQUIRE( (int)ptr->nest()->c() == 321 );

   ptr->nest()->c() = 345;
   REQUIRE( (int)ptr->nest()->c() == 345 );

   auto u = ptr.unpack();
   REQUIRE( u.a == 123 );
   REQUIRE( u.nest.a == 456 );
   REQUIRE( u.nest.b == 789 );
   REQUIRE( u.nest.c == 345 );

}
TEST_CASE("fracpack-nest") {
  REQUIRE( clio::can_memcpy<nested_final_struct>() == false );
  REQUIRE( clio::fracpack_size( simple_with_mult_string() ) == 22 );
  REQUIRE( clio::fracpack_size( nested_final_struct() ) == 30 );
  REQUIRE( clio::fracpack_size( nested_final_struct({.nest={.s="hello"}}) ) == 39 );
 
  nested_final_struct ex({.a=42, .nest={.s="hello", .s2 = "world"}});

  clio::frac_ptr<nested_final_struct> p(ex);
  std::cout << "p.size: " << p.size() <<"\n";

  REQUIRE( (uint32_t)p->a() == 42 );

  auto n = p->nest();
  auto np = &n;
  std::string strs = p->nest()->s();
  std::cout << "n.s: " << p->nest()->s() <<"\n";
  std::cout << "n.s2: " << p->nest()->s2() <<"\n";
  std::cout << "n.s: " << n->s() <<"\n";
  std::cout << "n.s: " << (*n).s() <<"\n";
  std::cout << "p.a: " << p->a() <<"\n";
  std::cout << "n.s: " << p->nest()->s() <<"\n";

  REQUIRE( p->nest()->s() == std::string_view("hello") );
  REQUIRE( p->nest()->s2() == std::string_view("world") );

  auto u = p.unpack();
  REQUIRE( u.a == 42 );
  REQUIRE( u.nest.s == std::string_view("hello") );
  REQUIRE( u.nest.s2 == std::string_view("world") );
}

TEST_CASE("fracpack") {
  std::cout << "starting test...\n";
  REQUIRE( sizeof(simple) == 14 );
  REQUIRE( clio::can_memcpy<simple>() == true );
  REQUIRE( clio::can_memcpy<simple>() == true );
  REQUIRE( clio::can_memcpy<simple_with_string>() == false );
  REQUIRE( clio::is_fixed_structure<simple>() == true );
  REQUIRE( clio::is_fixed_structure<not_final_simple>() == false );
  REQUIRE( clio::is_fixed_structure<ext_simple_with_string>() == false);

  REQUIRE( clio::can_memcpy<mixed_simple>() == false );
  REQUIRE( sizeof(req_align_simple) == 16 );
 // REQUIRE( clio::can_memcpy<req_align_simple>() == false );
  REQUIRE( clio::can_memcpy<not_final_simple>() == false );


  simple_with_string s{ .a = 65, .b = 100, .c = 300, .s = "hello"};
  auto size = clio::fracpack_size( s );

  std::cout <<"hello size: " << size<<"\n";
  REQUIRE( clio::fracpack_size(s) == 27 );

  std::vector<char> buffer(size);
  clio::fixed_buf_stream buf( buffer.data(), size );
  clio::fracpack( s, buf );


  REQUIRE( clio::fracpack_size( simple() ) == sizeof(simple) );
  REQUIRE( clio::fracpack_size( not_final_simple() ) == sizeof(simple)+2 );

  std::cout << "can memcpy simple: " << clio::can_memcpy<simple>() <<"\n";
  std::cout << "\n...\n";
  std::cout << "can memcpy mixed_simple: " <<clio::can_memcpy<mixed_simple>() <<"\n";

  /*
  clio::reflect<simple_with_string>::proxy<clio::frac_view_proxy>  prox( buffer.data() );
  REQUIRE( (uint64_t)prox.a() == 65 );
  REQUIRE( (uint64_t)prox.b() == 100 );
  REQUIRE( (uint64_t)prox.c() == 300 );
  std::cout << "a: " << prox.a() <<"\n";
  std::cout << "b: " << prox.b() <<"\n";
  std::cout << "c: " << prox.c() <<"\n";
  std::cout << "s: " << *prox.s() <<"\n";
  */


  /*
  {
  simple_with_mult_string s{ .a = 65, .b = 100, .c = 300, .s = "hello", .s2="world"};
  auto size = clio::fracpack_size( s );
  std::vector<char> buffer(size);
  clio::fixed_buf_stream buf( buffer.data(), size );
  clio::fracpack( s, buf );

  clio::reflect<simple_with_mult_string>::proxy<clio::frac_proxy>  prox( buffer.data() );
  std::cout << "a: " << prox.a() <<"\n";
  std::cout << "b: " << prox.b() <<"\n";
  std::cout << "c: " << prox.c() <<"\n";
  std::cout << "s: " << prox.s()->str() <<"\n";
  std::cout << "s2: " << prox.s2()->str() <<"\n";
  }
  */
}


/** extensibe, not aligned */
struct struct_with_vector_char {
  std::vector<char> test;
};
CLIO_REFLECT( struct_with_vector_char, test )

struct struct_with_vector_int{
  std::vector<uint32_t> test;
};
CLIO_REFLECT( struct_with_vector_int, test )

struct struct_with_vector_str final {
  std::vector<std::string> test;
};
CLIO_REFLECT( struct_with_vector_str, test )

TEST_CASE( "fracpack_vector" ) {
   clio::frac_ptr emptyp( struct_with_vector_char{ .test = {} } );
   REQUIRE( emptyp.size() == (2+4) );

   clio::frac_ptr p( struct_with_vector_char{ .test = { '1', '2', '3' } } );
   REQUIRE( p.size() == (2+4+4+3) );
   std::cout<< "size: " << p.size() <<"\n";

   std::cout << "test->size() = " << p->test()->size() <<"\n";
   REQUIRE( p->test()->size() == 3 );
   REQUIRE( p->test()[0] == '1' );
   REQUIRE( p->test()[1] == '2' );
   REQUIRE( p->test()[2] == '3' );
   std::cout << "[0] = " << p->test()[0] <<"\n";

   std::cout<<"\n\n--------------\n";
   auto u = p.unpack();
   REQUIRE( u.test.size() == 3 );
   REQUIRE( u.test[0] == '1' );
   REQUIRE( u.test[1] == '2' );
   REQUIRE( u.test[2] == '3' );
   std::cout<<"\n\n--------------\n";

   { /// testing with INT
   clio::frac_ptr p( struct_with_vector_int{ .test = { 1, 2, 3 } } );
   REQUIRE( p.size() == (2+4+4+3*4) );
   std::cout<< "size: " << p.size() <<"\n";

   std::cout << "test->size() = " << p->test()->size() <<"\n";
   REQUIRE( p->test()->size() == 3 );
   REQUIRE( p->test()[0] == 1 );
   REQUIRE( p->test()[1] == 2 );
   REQUIRE( p->test()[2] == 3 );
   std::cout << "[0] = " << p->test()[0] <<"\n";

   auto u = p.unpack();
   REQUIRE( u.test.size() == 3 );
   REQUIRE( u.test[0] == 1 );
   REQUIRE( u.test[1] == 2 );
   REQUIRE( u.test[2] == 3 );
   }
}

/**
 * 0   offset_ptr  vec = 4     
 *   ---struct heap ---
 * 4   uint32_t size = numelements*sizeof(offsetptr) = 2 * 4 = 8
 * 8   [0] = offset1 = 8
 * 12  [0] = offset2 = 4+4+2 = 10
 *   --- vec heap --
 * 16  uint32_t strlen = 2
 * 20  char[2]   "a|"
 * 22  uint32_t strlen = 3
 * 26  char[2]   "bc|"
 *
 */
TEST_CASE( "fracpack_vector_str" ) {
   clio::frac_ptr p( struct_with_vector_str{ .test = { "a|", "bc|" } } );
   std::cout<< "size: " << p.size() <<"\n";
   std::cout <<"vecptr       0]  4  == " << *reinterpret_cast<uint32_t*>(p.data()+0+4)<<"\n";
   std::cout <<"vecsize      4]  8  == " << *reinterpret_cast<uint32_t*>(p.data()+4+4)<<"\n";
   std::cout <<"veco[0]      8]  8  == " << *reinterpret_cast<uint32_t*>(p.data()+8+4)<<"\n";
   std::cout <<"veco[1]      12] 10 == " << *reinterpret_cast<uint32_t*>(p.data()+12+4)<<"\n";
   std::cout <<"str[0].size  16] 2  == " << *reinterpret_cast<uint32_t*>(p.data()+16+4)<<"\n";
   std::cout <<"str[1].size  26] 3  == " << *reinterpret_cast<uint32_t*>(p.data()+22+4)<<"\n";

   REQUIRE( p->test()->size() == 2 );
   REQUIRE( p->test()[0].size() == 2 );
   REQUIRE( p->test()[1].size() == 3 );
   std::cout << "\n"<<p->test()[0] <<"   " << p->test()[0].size()<<"\n";
   std::cout << "\n"<<p->test()[1] <<"   " << p->test()[1].size()<<"\n";

   auto u = p.unpack();
   REQUIRE( u.test.size() == 2 );
   REQUIRE( u.test[0].size() == 2 );
   REQUIRE( u.test[1].size() == 3 );
   REQUIRE( u.test[0] == "a|" );
   REQUIRE( u.test[1] == "bc|" );
}

struct inner final {
    uint32_t a;
    uint32_t b;
};
CLIO_REFLECT( inner, a, b )
struct outer final {
   inner in;
   uint32_t c;
};
CLIO_REFLECT( outer, in, c )

TEST_CASE( "nestfinal" ) {
   REQUIRE( clio::can_memcpy<outer>() == true );

   clio::frac_ptr<outer> p( {.in = { .a = 1, .b = 2 }, .c= 3 } );
   std::cout << p->in()->a() <<"\n";
   std::cout << p->in()->b() <<"\n";

   auto u = p.unpack();
   REQUIRE( u.in.a == 1 );
   REQUIRE( u.in.b == 2 );
   REQUIRE( u.c == 3 );
}
struct vecstruct {
   std::vector<outer> vs;
};
CLIO_REFLECT( vecstruct, vs )


TEST_CASE( "vectorstruct" ) {
  vecstruct test{ .vs = { {.c = 1}, {.c=2}, {.c=3} }  };
  clio::frac_ptr<vecstruct> p(test);
  REQUIRE( p.size() == 2+4+4+3*12 );
  REQUIRE( (int)p->vs()[0].c() == 1 );
  REQUIRE( (int)p->vs()[1].c() == 2 );
  REQUIRE( (int)p->vs()[2].c() == 3 );

  auto u = p.unpack();
  REQUIRE( u.vs[0].c == 1 );
  REQUIRE( u.vs[1].c == 2 );
  REQUIRE( u.vs[2].c == 3 );
}

struct tree {
   uint32_t i;
   std::string s;
   std::vector<tree> children;
};
CLIO_REFLECT( tree, i, s, children )

TEST_CASE( "tree" ) {
   tree t{
      .i = 1,
      .s = "one",
      .children  = {
          { .i = 2, 
            .s = "two",
            .children = {
               {.i = 3, .s = "three"}
            } 
          }
      }
   };
  clio::frac_ptr<tree> p(t);
  REQUIRE( (uint32_t)p->i() == 1 );
  REQUIRE( (std::string_view)p->s() == "one" );
  REQUIRE( (uint32_t)p->children()[0].i() == 2 );
  REQUIRE( (std::string_view)p->children()[0].s() == "two" );
  REQUIRE( (uint32_t)p->children()[0].children()[0].i() == 3 );
  REQUIRE( (std::string_view)p->children()[0].children()[0].s() == "three" );
  std::cout << "size: " << p.size() <<"\n";

  auto u = p.unpack();
  REQUIRE( u.i == 1 );
  REQUIRE( u.s == "one" );
  REQUIRE( u.children[0].i == 2 );
  REQUIRE( u.children[0].s == "two" );
  REQUIRE( u.children[0].children[0].i == 3 );
  REQUIRE( u.children[0].children[0].s == "three" );

}


struct optstr final {
   std::optional<std::string> str;
};

CLIO_REFLECT( optstr, str );

TEST_CASE( "optstr" ) {
   {
      REQUIRE( clio::fracpack_size(optstr{}) == 4 );
      std::cout <<"---packing---\n";   
      clio::frac_ptr<optstr> p(optstr{});
      REQUIRE( not p->str().valid() );
      auto u = p.unpack();
      REQUIRE( !u.str );
   }

   {
      clio::frac_ptr<optstr> p(optstr{.str="hello"});
      REQUIRE( p.size() == 13 ); /// offset+size+"hello"
      REQUIRE( (std::string_view)p->str() == "hello" );

      auto u = p.unpack();
      REQUIRE( !!u.str );
      REQUIRE( *u.str == "hello" );
   }
}

struct ext_v1 {
   std::string a;
   std::optional<std::string> b;
   std::optional<std::string> c;
};
CLIO_REFLECT( ext_v1, a, b, c )

struct ext_v2 {
   std::string a;
   std::optional<std::string> b;
   std::optional<std::string> c;
   std::optional<std::string> d;
   std::optional<std::string> e;
};
CLIO_REFLECT( ext_v2, a, b, c, d, e );


TEST_CASE( "extend" ) {
   clio::frac_ptr<ext_v1> v1({"hello", "world", "again"});
   REQUIRE( v1.size() == 2+4+4+5+4+4+5+4+4+5 );
   REQUIRE( (std::string_view)v1->c() == "again" );


   REQUIRE( (std::string_view)v1->a() == "hello" );
   REQUIRE( v1->b().valid() );
   REQUIRE( (std::string_view)v1->b() == "world" );
   REQUIRE( v1->c().valid() );


   clio::frac_ptr<ext_v2> v2( v1.data() );
   REQUIRE( (std::string_view)v2->a() == "hello" );
   REQUIRE( v2->b().valid() );

   REQUIRE( (std::string_view)v1->b() == "world" );
   REQUIRE( (std::string_view)v2->b() == "world" );
   REQUIRE( (std::string_view)v1->c() == "again" );
   std::cout << "v1->c: " << (std::string_view)v1->c() <<"\n";
   REQUIRE( v2->c().valid() );
   REQUIRE( (std::string_view)v2->c() == "again" );
   REQUIRE( !v2->d().valid() );
   REQUIRE( !v2->e().valid() );



   clio::frac_ptr<ext_v2> v2a({"hello", "world", "again", "and", "extra"});
   REQUIRE( v2a->d().valid() );
   REQUIRE( (std::string_view)v2a->d() == "and" );
   REQUIRE( v2a->e().valid() );
   REQUIRE( (std::string_view)v2a->e() == "extra" );

   clio::frac_ptr<ext_v1> v1a( v2a.data() );
   REQUIRE( (std::string_view)v1a->c() == "again" );
   REQUIRE( (std::string_view)v1a->a() == "hello" );
   REQUIRE( v1a->b().valid() );
   REQUIRE( (std::string_view)v1a->b() == "world" );
   REQUIRE( v1a->c().valid() );

   auto v1au = v1a.unpack();
   auto v2au = v2a.unpack();
   auto v1u = v1.unpack();
   auto v2u = v2.unpack();

}


struct transfer {
   uint32_t from;
   uint32_t to;
   uint64_t amount;
   std::string memo;
};
CLIO_REFLECT( transfer, from, to, amount, memo )

struct action {
   uint32_t sender;
   uint32_t contract;
   uint16_t act;
   std::vector<char> data;

   template<typename T>
   void set( const T& a ) {
      data.resize(clio::fracpack_size(a));
      clio::fixed_buf_stream buf( data.data(), data.size() ); 
      clio::fracpack( a, buf);
   }
   action(){}
   template<typename T>
   action( uint32_t s, uint32_t c, uint32_t a, const T& t )
   :sender(s),contract(c),act(a){ set(t); }
};
CLIO_REFLECT( action, sender, contract, act, data )

struct transaction {
   uint32_t        expires;
   uint16_t        tapos;
   uint16_t        flags;
   std::vector<action>  actions;
};
CLIO_REFLECT( transaction, expires, tapos, flags, actions )


TEST_CASE( "blockchain" )
{
   contract::transferT trobj{ .from = 7, .to = 8, .amount=42, .memo="test" };
   transfer frtr{ .from = 7, .to = 8, .amount=42, .memo="test" };

   double gt = 1;
   double ft = 1;
   {
      auto start = std::chrono::steady_clock::now();
      for( uint32_t i = 0; i < 10000; ++i ) {
         flatbuffers::FlatBufferBuilder fbb;
         fbb.Finish(contract::transfer::Pack(fbb, &trobj));
      }
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      gt = std::chrono::duration<double, std::milli>(delta).count() ;
      std::cout << "google pack:     " << gt <<" ms\n";
   }
   {
      auto start = std::chrono::steady_clock::now();
      for( uint32_t i = 0; i < 10000; ++i ) {
         auto tmp = clio::frac_ptr<transfer>(frtr);
      }
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      ft = std::chrono::duration<double, std::milli>(delta).count() ;
      std::cout << "fracpack pack:   " << ft <<" ms  " << 100 * ft / gt <<"%\n";
   }

   {
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Finish(contract::transfer::Pack(fbb, &trobj));

      auto start = std::chrono::steady_clock::now();
      for( uint32_t i = 0; i < 10000; ++i ) {
         contract::transferT temp;
         contract::Gettransfer(fbb.GetBufferPointer())->UnPackTo( &temp);
         REQUIRE( temp.memo == "test" );
      }
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      gt = std::chrono::duration<double, std::milli>(delta).count(); 
      std::cout << "google unpack:   " << gt <<" ms\n";
   }
   {
      auto tmp = clio::frac_ptr<transfer>(frtr);
      auto start = std::chrono::steady_clock::now();
      for( uint32_t i = 0; i < 10000; ++i ) {
         auto o = tmp.unpack();
         REQUIRE( o.memo == "test" );
      }
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      ft = std::chrono::duration<double, std::milli>(delta).count() ;
      std::cout << "fracpack unpack: " << ft <<" ms  " << 100 * ft / gt << "%\n";
   }
   
   {
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Finish(contract::transfer::Pack(fbb, &trobj));

      auto start = std::chrono::steady_clock::now();
      uint64_t x = 0;
      for( uint32_t i = 0; i < 10000; ++i ) {
         auto t = contract::Gettransfer(fbb.GetBufferPointer());
         x += t->from() + t->to() + t->amount() + t->memo()->size();
      }
         REQUIRE( x == 10000*(7+8+42+4) );
         //REQUIRE( std::string_view(t->memo()->c_str()) == std::string_view("test") );
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      gt = std::chrono::duration<double, std::milli>(delta).count();
      std::cout << "google read:     " << gt <<" ms\n";
   }
   {
      auto tmp = clio::frac_ptr<transfer>(frtr);
      auto start = std::chrono::steady_clock::now();
      uint64_t x = 0;
      for( uint32_t i = 0; i < 10000; ++i ) {
         x += (uint64_t)tmp->from() + (uint64_t)tmp->to() + (uint64_t)tmp->amount() + tmp->memo()->size();
      }
         REQUIRE( x == 10000*(7+8+42+4) );
         //REQUIRE( std::string_view(tmp->memo()) == std::string_view("test") );
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      ft = std::chrono::duration<double, std::milli>(delta).count() ;
      std::cout << "fracpack read:   " << ft <<" ms  "<<100*ft/gt <<"%\n";
   }

   {
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Finish(contract::transfer::Pack(fbb, &trobj));

      auto start = std::chrono::steady_clock::now();
      uint64_t x = 0;
      for( uint32_t i = 0; i < 10000; ++i ) {
         auto t = contract::Gettransfer(fbb.GetBufferPointer());
         flatbuffers::Verifier v( fbb.GetBufferPointer(), fbb.GetSize() );
         t->Verify( v );
      }
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      gt = std::chrono::duration<double, std::milli>(delta).count(); 
      std::cout << "google check:    " << gt <<" ms\n";
   }

   {
      auto tmp = clio::frac_ptr<transfer>(frtr);
      auto start = std::chrono::steady_clock::now();
      uint64_t x = 0;
      for( uint32_t i = 0; i < 10000; ++i ) {
           tmp.validate();
      }
         //REQUIRE( std::string_view(tmp->memo()) == std::string_view("test") );
      auto end = std::chrono::steady_clock::now();
      auto delta = end - start;
      ft = std::chrono::duration<double, std::milli>(delta).count() ;
      std::cout << "fracpack check:  " << ft <<" ms  "<<100*ft/gt<<"%\n";
   }



   flatbuffers::FlatBufferBuilder trans_builder(1024);
   contract::transferBuilder  trb(trans_builder);
   trb.add_from( 7 );
   trb.add_to( 8 );
   trb.add_amount( 42 );
   trb.add_memo( trans_builder.CreateString( "test" ) );
   auto finishedtrb = trb.Finish();
   auto transfer_size = trans_builder.GetSize();


   flatbuffers::FlatBufferBuilder builder(1024);
   cliofb::actionBuilder a(builder);
   a.add_sender(4);
   a.add_contract(5);
   a.add_act(6);
   auto a1 = a.Finish();

   cliofb::transactionBuilder b(builder);
   b.add_tapos(2);
   b.add_flags(3);
   b.add_expire(123);

   std::vector<flatbuffers::Offset<cliofb::action>> acts{a1};
   auto av = builder.CreateVector(acts);

   b.add_actions( av );
   //auto act = builder.Createaction( 4, 5, 6 );
   
   auto fbt =  b.Finish();

   uint8_t *buf = builder.GetBufferPointer();
   int size = builder.GetSize();

   transaction t{ .expires = 123, .tapos = 2, .flags = 3, 
                  .actions = {
                     action( 4,5,6, transfer{ .from=7, .to=8, .amount=42, .memo="test" }),
                     action( 14,15,16, transfer{ .from=17, .to=18, .amount=142, .memo="test" })
                  }
                };
   clio::frac_ptr<transaction> tp( t );
   auto t2 = tp.unpack();
   std::cout << "size: " << tp.size() <<"\n";

   {
//   clio::size_stream ss;
//   clio::to_bin(t, ss);
//   std::vector<char> buf(ss.size);
//   clio::fixed_buf_stream ps(buf.data(), buf.size());
   auto buf = clio::to_bin(t);

   std::cout << "bin size: "<<buf.size()<<"\n";
   std::cout <<"fbsize: " << size <<"\n";
   std::cout <<"transfersize: " << transfer_size << "  vs frac " << clio::fracpack_size( transfer{.memo="test"} ) <<"\n";
   }
}


/*
struct inner {
   int b;
};

struct st {
  int  a;
  inner in;
}

struct st_proxy {
    char* data;
    IntWrapper a()const {
      return IntWrapper( data );
    }
    inner_proxy in()const {
       return inner_proxy( data+sizeof(int) );
    }
};

struct inner_proxy {
  char* data;
  IntWrapper b()const {
     return IntWrapper(data);
  }
}

st_proxy p;

p.in().b()
*/















