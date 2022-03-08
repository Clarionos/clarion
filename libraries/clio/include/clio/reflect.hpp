#pragma once
#include <vector>
#include <optional>
#include <string_view>
#include <variant>
#include <tuple>
#include <type_traits>
#include <map>

#include <boost/core/demangle.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/facilities/overload.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/variadic/to_tuple.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <clio/member_proxy.hpp>
#include <clio/get_type_name.hpp>

namespace clio {

    template<typename R, typename C, typename... Args>
    std::tuple<std::decay_t<Args>...> args_as_tuple( R (C::*)(Args...) );
    template<typename R, typename C, typename... Args>
    std::tuple<std::decay_t<Args>...> args_as_tuple( R (C::*)(Args...)const );

    template<typename R, typename C, typename... Args>
    R result_of( R (C::*)(Args...)const );
    template<typename R, typename C, typename... Args>
    R result_of( R (C::*)(Args...) );

    template<typename R, typename C>
    constexpr R result_of_member( R (C::*) );
    template<typename R, typename C>
    constexpr C class_of_member( R (C::*) );

    template<typename R, typename C, typename... Args>
    void result_of_member( R (C::*)(Args...)const );
    template<typename R, typename C, typename... Args>
    void result_of_member( R (C::*)(Args...) );


    struct meta {
        const char*                        name;
        int32_t                            number;
        uint64_t                           offset = 0;
        std::initializer_list<const char*> param_names;
    };

 #define CLIO_REFLECT_ARGS_INTERNAL( r, OP, i, PARAM) \
   BOOST_PP_COMMA_IF(i) BOOST_PP_STRINGIZE(PARAM)

 #define CLIO_REFLECT_ARGS_HELPER( METHOD, PARAM_NAMES ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_ARGS_INTERNAL, METHOD, PARAM_NAMES ) 


 #define CLIO_REFLECT_FILTER_PARAMS(NAME, IDX, ...) { CLIO_REFLECT_ARGS_HELPER( METHOD, BOOST_PP_VARIADIC_TO_SEQ( __VA_ARGS__ ) ) }
 #define CLIO_REFLECT_FILTER_NAME(NAME, IDX, ...) NAME
 #define CLIO_REFLECT_FILTER_NAME_STR(NAME, IDX, ...) BOOST_PP_STRINGIZE(NAME) 
 #define CLIO_REFLECT_FILTER_IDX(NAME, IDX, ...) IDX


 #define CLIO_REFLECT_FOREACH_PB_INTERNAL( r, OP, member) \
       { auto off = __builtin_offsetof(OP,member); \
      lambda( clio::meta{ .number = CLIO_REFLECT_FILTER_IDX member, \
                    .name   = CLIO_REFLECT_FILTER_NAME_STR member, \
                    .offset = off,\
                    .param_names = CLIO_REFLECT_FILTER_PARAMS member }, \
                    &OP::CLIO_REFLECT_FILTER_NAME member);}\

      
 #define CLIO_REFLECT_FOREACH_INTERNAL( r, OP, i, member) \
       { auto off = __builtin_offsetof(OP,member); \
      (void)lambda( clio::meta{ \
                          .name =  BOOST_PP_STRINGIZE(member), \
                          .number = i+1 , .offset = off \
                         }, \
              &OP::member);}\

 #define CLIO_REFLECT_MEMBER_BY_STR_INTERNAL( r, OP, member) \
   if( BOOST_PP_STRINGIZE(member) == m ) {            \
       (void)lambda( &OP::member ); return true;                 \
   }

 #define CLIO_REFLECT_MEMBER_BY_STR_PB_INTERNAL( r, OP, member) \
   if( CLIO_REFLECT_FILTER_NAME_STR member == m ) {            \
       (void)lambda( &OP::CLIO_REFLECT_FILTER_NAME member ); return true;                 \
   }


 #define CLIO_REFLECT_MEMBER_BY_IDX_PB_INTERNAL( r, OP, member) \
   case CLIO_REFLECT_FILTER_IDX member: (void)lambda( &OP:: CLIO_REFLECT_FILTER_NAME member); return true;

#define CLIO_REFLECT_PROXY_MEMBER_BY_IDX_INTERNAL( r, OP, I, member ) \
    template<typename...Args> \
    auto member( Args&&... args ) { \
        return _clio_proxy_obj.call( clio::meta{ .number = I+1, \
                                          .name = BOOST_PP_STRINGIZE(member)\
                                        }, \
                             &OP::member, std::forward<Args>(args)... ); \
    } \


#define CLIO_REFLECT_SMPROXY_MEMBER_BY_IDX_INTERNAL( r, OP, I, member ) \
    clio::member_proxy<I,clio::hash_name(BOOST_PP_STRINGIZE(member)),&OP::member,ProxyObject> member(){ return _clio_proxy_obj; }\
    clio::member_proxy<I,clio::hash_name(BOOST_PP_STRINGIZE(member)),&OP::member,const ProxyObject> member()const{ return _clio_proxy_obj; } 


#define CLIO_REFLECT_SMPROXY_MEMBER_BY_IDX_HELPER( QUERY_CLASS, MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_SMPROXY_MEMBER_BY_IDX_INTERNAL, QUERY_CLASS, MEMBER_IDXS )


#define CLIO_REFLECT_PROXY_MEMBER_BY_PB_INTERNAL( r, OP, member ) \
    template<typename...Args> \
    auto CLIO_REFLECT_FILTER_NAME member ( Args&&... args ) { \
        return _clio_proxy_obj.call( clio::meta{ .number = CLIO_REFLECT_FILTER_IDX member, \
                                          .name   = CLIO_REFLECT_FILTER_NAME_STR member, \
                                          .param_names = CLIO_REFLECT_FILTER_PARAMS member }, \
                              &OP::CLIO_REFLECT_FILTER_NAME member, std::forward<Args>(args)... ); \
    } 


 #define CLIO_REFLECT_FOREACH_MEMBER_HELPER( QUERY_CLASS,  MEMBERS ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_FOREACH_INTERNAL, QUERY_CLASS, MEMBERS )

 #define CLIO_REFLECT_MEMBER_BY_STR_HELPER( QUERY_CLASS,  MEMBERS ) \
    BOOST_PP_SEQ_FOR_EACH( CLIO_REFLECT_MEMBER_BY_STR_INTERNAL, QUERY_CLASS, MEMBERS )


 #define CLIO_REFLECT_MEMBER_BY_IDX_I_INTERNAL( r, OP, I, member) \
   case I+1: (void)lambda( &OP::member ); return true;

 #define CLIO_REFLECT_MEMBER_BY_IDX_HELPER( QUERY_CLASS,  MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_MEMBER_BY_IDX_I_INTERNAL, QUERY_CLASS, MEMBER_IDXS )


 #define CLIO_REFLECT_MEMBER_BY_NAME_I_INTERNAL( r, OP, I, member) \
   case clio::hash_name(BOOST_PP_STRINGIZE(member)): (void)lambda( &OP::member ); return true;

 #define CLIO_REFLECT_MEMBER_BY_NAME_HELPER( QUERY_CLASS,  MEMBER_NAMES ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_MEMBER_BY_NAME_I_INTERNAL, QUERY_CLASS, MEMBER_NAMES )

 #define CLIO_REFLECT_MEMBER_TYPE_BY_IDX_INTERNAL( r, OP, I,  member ) \
     BOOST_PP_COMMA_IF( I )  std::decay_t< decltype(clio::result_of_member(&OP::member))> 


 #define CLIO_REFLECT_MEMBER_TYPE_IDX_HELPER( QUERY_CLASS,  MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_MEMBER_TYPE_BY_IDX_INTERNAL, QUERY_CLASS, MEMBER_IDXS )


 #define CLIO_REFLECT_FOREACH_MEMBER_PB_HELPER( QUERY_CLASS,  MEMBERS ) \
    BOOST_PP_SEQ_FOR_EACH( CLIO_REFLECT_FOREACH_PB_INTERNAL, QUERY_CLASS, MEMBERS )

 #define CLIO_REFLECT_MEMBER_INDEX_HELPER( QUERY_CLASS,  MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_MEMBER_PB_INTERNAL, QUERY_CLASS, MEMBER_IDXS )

 #define CLIO_REFLECT_MEMBER_BY_STR_PB_HELPER( QUERY_CLASS,  MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH( CLIO_REFLECT_MEMBER_BY_STR_PB_INTERNAL, QUERY_CLASS, MEMBER_IDXS )

 #define CLIO_REFLECT_MEMBER_BY_IDX_PB_HELPER( QUERY_CLASS,  MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH( CLIO_REFLECT_MEMBER_BY_IDX_PB_INTERNAL, QUERY_CLASS, MEMBER_IDXS )
 
#define CLIO_REFLECT_PROXY_MEMBER_BY_IDX_HELPER( QUERY_CLASS, MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_PROXY_MEMBER_BY_IDX_INTERNAL, QUERY_CLASS, MEMBER_IDXS )

#define CLIO_REFLECT_PROXY_MEMBER_BY_PB_HELPER( QUERY_CLASS, MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH( CLIO_REFLECT_PROXY_MEMBER_BY_PB_INTERNAL, QUERY_CLASS, MEMBER_IDXS )



 #define CLIO_REFLECT_PARAMS_BY_IDX_PB_INTERNAL( r, OP, i, member) \
   if constexpr ( std::is_member_function_pointer_v<decltype( &OP:: CLIO_REFLECT_FILTER_NAME member )> ) \
       return OP:: BOOST_PP_CAT( CLIO_REFLECT_FILTER_NAME member ,___PARAM_NAMES);

 #define CLIO_REFLECT_PARAMS_BY_IDX_PB_HELPER( QUERY_CLASS, MEMBER_IDXS ) \
    BOOST_PP_SEQ_FOR_EACH_I( CLIO_REFLECT_PARAMS_BY_IDX_PB_INTERNAL, QUERY_CLASS, MEMBER_IDXS )


                                        /*
       template<typename ProxyObject> \
       struct proxy { \
            template<typename... Args> \
            proxy( Args&&... args ):_clio_proxy_obj( std::forward<Args>(args)... ){}\
            ProxyObject* operator->(){ return &_clio_proxy_obj; } \
            const ProxyObject* operator->()const{ return &_clio_proxy_obj; } \
            ProxyObject& operator*(){ return _clio_proxy_obj; } \
            const ProxyObject& operator*()const{ return _clio_proxy_obj; } \
            CLIO_REFLECT_PROXY_MEMBER_BY_IDX_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
           private: \
             ProxyObject _clio_proxy_obj; \
       }; \
       */


#define CLIO_REFLECT(QUERY_CLASS, ...)            \
   CLIO_REFLECT_TYPENAME( QUERY_CLASS )           \
   struct reflect_impl_##QUERY_CLASS {            \
      static constexpr bool is_defined = true;    \
      static constexpr bool is_struct  = true;    \
      static inline constexpr const char* name() { return BOOST_PP_STRINGIZE(QUERY_CLASS); }  \
      typedef std::tuple< \
            CLIO_REFLECT_MEMBER_TYPE_IDX_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) > struct_tuple_type;     \
      template <typename L> constexpr inline static void for_each(L&& lambda) {                  \
         CLIO_REFLECT_FOREACH_MEMBER_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))  \
      }                                                                                          \
      template <typename L> inline static bool get(const std::string_view& m, L&& lambda) {      \
         CLIO_REFLECT_MEMBER_BY_STR_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))   \
          return false; \
      }                                                                                           \
      template <typename L> inline static bool get(int64_t m, L&& lambda) {                       \
         switch (m) {                                                                             \
            CLIO_REFLECT_MEMBER_BY_IDX_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
         }                                                                                        \
         return false; \
      }                                                                                           \
      template <typename L> inline static bool get_by_name(uint64_t n, L&& lambda) {               \
         switch (n) {                                                                             \
            CLIO_REFLECT_MEMBER_BY_NAME_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
         }                                                                                        \
         return false; \
      }                                                                                           \
       template<typename ProxyObject> \
       struct proxy { \
          private: \
            ProxyObject _clio_proxy_obj; \
          public: \
            template<typename... Args> \
            proxy( Args&&... args ):_clio_proxy_obj( std::forward<Args>(args)... ){}\
            ProxyObject* operator->(){ return &_clio_proxy_obj; } \
            const ProxyObject* operator->()const{ return &_clio_proxy_obj; } \
            ProxyObject& operator*(){ return _clio_proxy_obj; } \
            const ProxyObject& operator*()const{ return _clio_proxy_obj; } \
            CLIO_REFLECT_SMPROXY_MEMBER_BY_IDX_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
       }; \
   }; \
   reflect_impl_##QUERY_CLASS get_reflect_impl(const QUERY_CLASS&);


#define CLIO_REFLECT_PB(QUERY_CLASS, ...)                                                                           \
   CLIO_REFLECT_TYPENAME( QUERY_CLASS ) \
   struct reflect_impl_##QUERY_CLASS {                                                                            \
      static constexpr bool is_defined = true;                                                                         \
      static constexpr bool is_struct  = true;                                                                        \
      static inline constexpr const char* name() { return BOOST_PP_STRINGIZE(QUERY_CLASS); }                                              \
      template <typename L> inline static void for_each(L&& lambda) {                                                  \
         CLIO_REFLECT_FOREACH_MEMBER_PB_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                         \
      }                                                                                                                \
      template <typename L> inline static bool get(const std::string_view& m, L&& lambda) {                            \
            CLIO_REFLECT_MEMBER_BY_STR_PB_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                              \
         return false; \
      }   \
      template <typename L> inline static bool get(int64_t m, L&& lambda) {                            \
         switch (m) {                                                                                  \
            CLIO_REFLECT_MEMBER_BY_IDX_PB_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                              \
         }                                                                                                             \
         return false; \
      } \
   };                                                                                                                  \
   reflect_impl_##QUERY_CLASS get_reflect_impl(const QUERY_CLASS&); \


#define CLIO_REFLECT_TEMPLATE_OBJECT(QUERY_CLASS, TPARAM, ...)                                                                  \
   template <typename T> struct reflect_impl_##QUERY_CLASS {                                                      \
      static constexpr bool is_defined = true;                                                                         \
      static constexpr bool is_struct  = true;                                                                        \
      static inline const char* name() { return BOOST_PP_STRINGIZE(QUERY_CLASS) "<" BOOST_PP_STRINGIZE(TPARAM) ">"; }           \
      template <typename L> inline static void for_each(L&& lambda) {                                                  \
         CLIO_REFLECT_FOREACH_MEMBER_HELPER(QUERY_CLASS<T>, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                      \
      }                                                                                                                \
      template <typename L> inline static void get(const std::string_view& m, L&& lambda) {                            \
          CLIO_REFLECT_MEMBER_HELPER(QUERY_CLASS<T>, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                          \
         return false; \
      }                                                                                                                \
      template <typename L> inline static void get(int64_t m, L&& lambda) {                                            \
         switch (m) {                                                                                                  \
            CLIO_REFLECT_MEMBER_INDEX_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                       \
         }                                                                                                             \
         return false; \
      }                                                                                                                \
      using tuple_type = std::tuple< \
            CLIO_REFLECT_MEMBER_TYPE_HELPER(QUERY_CLASS, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) >                      \
   };                                                                                                                  \
   template <typename T> reflect_impl_##QUERY_CLASS<T> get_reflect_impl(const QUERY_CLASS<T>&); \
   constexpr const char* get_type_name( QUERY_CLASS<T>* ) { return reflect_impl_##QUERY_CLASS :: name(); }

   template<typename QueryClass>
   struct reflect_undefined {
      static constexpr bool is_defined = false;
      static constexpr bool is_struct  = false;
      template<typename L> 
      static void get( const std::string_view& m, L&& lambda );
   };



                                        /*
   using std::string;
   CLIO_REFLECT_TYPENAME( int32_t )
   CLIO_REFLECT_TYPENAME( int64_t )
   CLIO_REFLECT_TYPENAME( int16_t )
   CLIO_REFLECT_TYPENAME( int8_t )
   CLIO_REFLECT_TYPENAME( uint32_t )
   CLIO_REFLECT_TYPENAME( uint64_t )
   CLIO_REFLECT_TYPENAME( uint16_t )
   CLIO_REFLECT_TYPENAME( uint8_t )
   CLIO_REFLECT_TYPENAME( float )
   CLIO_REFLECT_TYPENAME( double )
   CLIO_REFLECT_TYPENAME( char )
   CLIO_REFLECT_TYPENAME( bool )
   CLIO_REFLECT_TYPENAME( string )
   */


   template<typename QueryClass>
   reflect_undefined<QueryClass> get_reflect_impl(const QueryClass&);


   template<typename QueryClass>
   using reflect = std::decay_t<decltype(get_reflect_impl(std::declval<QueryClass>()))>;

   template<typename>
   struct is_std_vector : std::false_type {};

   template<typename T, typename A>
   struct is_std_vector<std::vector<T,A>> : std::true_type { using value_type = T; };

   template<typename>
   struct is_std_optional : std::false_type {};

   template<typename T>
   struct is_std_optional<std::optional<T>> : std::true_type { using value_type = T; };

   template<typename>
   struct is_std_variant : std::false_type {};

   template<typename... T>
   struct is_std_variant<std::variant<T...>> : std::true_type {
       static std::string name() {
           return get_variant_typename<T...>();
       }
       template<typename First, typename ...Rest>
       static std::string get_variant_typename() {
           if constexpr ( sizeof...(Rest) > 0 )
               return std::string( get_type_name<First>() ) + "|" + get_variant_typename<Rest...>();
           else
               return std::string( get_type_name<First>() ) + "|"; 
       }
       using alts_as_tuple = std::tuple<T...>;
   };

   template<typename>
   struct is_std_tuple : std::false_type {};

   template<typename... T>
   struct is_std_tuple<std::tuple<T...>> : std::true_type {
       static std::string name() {
           return get_tuple_typename<T...>();
       }
       template<typename First, typename ...Rest>
       static std::string get_tuple_typename() {
           if constexpr ( sizeof...(Rest) > 0 )
               return std::string( get_type_name<First>() ) + "&" + get_tuple_typename<Rest...>();
           else
               return std::string( get_type_name<First>() ) + "&"; 
       }
   };


   template<typename>
   struct is_std_map : std::false_type {};

   template<typename K, typename V>
   struct is_std_map<std::map<K,V>> : std::true_type {
       static const std::string& name() {
           static std::string n = std::string("map<") + get_type_name<K>() +"," + get_type_name<V>() + ">";
           return n;
       }
   };


} /// namespace clio

namespace std {
    namespace {
       CLIO_REFLECT_TYPENAME( string )
    }
}

