#pragma once
#include <tuple>

namespace clio {
enum class tuple_error {
   no_error,
   invalid_tuple_index
}; // tuple_error
} // namespace clio 

namespace std {
    template <>
    struct is_error_code_enum<clio::tuple_error> : true_type {};
} // namespace std


namespace clio {

class tuple_error_category_type : public std::error_category {
 public:
   const char* name() const noexcept override final { return "ConversionError"; }

   std::string message(int c) const override final {
      switch (static_cast<tuple_error>(c)) {
            // clang-format off
         case tuple_error::no_error:                 return "No error";
         case tuple_error::invalid_tuple_index:      return "invalid tuple index";
         default: return "unknown";
      };
   }
};

inline const tuple_error_category_type& tuple_error_category() {
   static tuple_error_category_type c;
   return c;
}

inline std::error_code make_error_code(tuple_error e) { return { static_cast<int>(e), tuple_error_category() }; }


    template <int N, typename T, typename L>
    void tuple_get(T& obj, int pos, L&& lambda) {
       if constexpr (N < std::tuple_size_v<T>) {
          if( N == pos ) {
              lambda( std::get<N>(obj) );
          }
          else tuple_get<N + 1>(obj, pos, std::forward<L>(lambda) );
       } else {
          throw_error(tuple_error::invalid_tuple_index);
       }
    }

    template <typename... T, typename L>
    void tuple_get(std::tuple<T...>& obj, int pos, L&& lambda) {
       tuple_get<0>(obj, pos, std::forward<L>(lambda) );
    }

    template <int N, typename T, typename L>
    void tuple_for_each(T& obj, L&& lambda) {
       if constexpr (N < std::tuple_size_v<T>) {
          lambda( N, std::get<N>(obj) );
          tuple_for_each<N + 1>(obj, std::forward<L>(lambda) );
       } 
       
    }

    template <typename... T, typename L>
    void tuple_for_each(std::tuple<T...>& obj, L&& lambda) {
       tuple_for_each<0>(obj, std::forward<L>(lambda) );
    }

    template <int N, typename T, typename L>
    void tuple_for_each(const T& obj, L&& lambda) {
       if constexpr (N < std::tuple_size_v<T>) {
          lambda( N, std::get<N>(obj) );
          tuple_for_each<N + 1>(obj, std::forward<L>(lambda) );
       } 
    }

    template <typename... T, typename L>
    void tuple_for_each(const std::tuple<T...>& obj, L&& lambda) {
       tuple_for_each<0>(obj, std::forward<L>(lambda) );
    }

}
