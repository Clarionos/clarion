#pragma once

namespace clio {

    template<uint32_t I, uint64_t Name, auto mptr, typename ProxyObject>
    struct member_proxy {
         /**
          *  This object is created on a type created by the macro, I represents the member number in the
          *  containing type. The containing type's first member is a ProxyObject. This function does the
          *  pointer math necessary to find the ProxyObject.
          *
          *  Alternatively the macro code would have to initialize every member_proxy with this, which would
          *  bloat the size of the member_proxy object
          *  
          *  flat_view<T> => reflect<T>::member_proxy<proxy_impl>
          *
          *  struct member_proxy<proxy_impl>  {
          *     proxy_impl proxy___;
          *     member_proxy<0, ptr, proxy_impl> member_zero
          *     member_proxy<1, ptr, proxy_impl> member_one
          *     member_proxy<2, ptr, proxy_impl> member_two
          *  }
          *  a packed flat buffer.  
          *
          *  Let char* buf = point to a flat buffer;
          *  Let reinterpet buf as memper_proxy<proxy_impl>*, this makes the address of proxy__ equal to
          *  the address of member_proxy<T> because it is the first element. 
          *
          *  because member_proxy has no values it takes 1 byte in member_proxy and the value of that byte
          *  is never read by member_proxy... member_proxy always gets the address of proxy___ and then
          *  does offset math.
          *
          */
         constexpr auto proxyptr()const { 
             return (reinterpret_cast<const ProxyObject*>( reinterpret_cast<const char*>(this)-sizeof(*this)*(I+1)) );
         }
         constexpr auto proxyptr(){ 
             return (reinterpret_cast<ProxyObject*>( reinterpret_cast<char*>(this)-sizeof(*this)*(I+1)) );
         }
         constexpr const auto& get()const { return *(proxyptr()->template get<I,Name,mptr>()); }
         constexpr auto& get() { return *(proxyptr()->template get<I,Name,mptr>()); }

         template<typename... Ts>
         constexpr auto operator()(Ts&&... args) { 
             return proxyptr()->template call<I,Name,mptr>( std::forward<Ts>(args)...); 
         }
         template<typename... Ts>
         constexpr auto operator()(Ts&&... args)const { 
             return proxyptr()->template call<I,Name,mptr>( std::forward<Ts>(args)...); 
         }

         constexpr auto operator->(){ return (proxyptr()->template get<I,Name,mptr>()); }
         constexpr const auto operator->()const { return (proxyptr()->template get<I,Name,mptr>()); }

         constexpr auto& operator*(){ return get(); }
         constexpr const auto& operator*()const { return get(); }

         template<typename T>
         constexpr auto& operator[]( T&& k ) { return get()[ std::forward<T>(k)]; }

         template<typename T>
         constexpr const auto& operator[]( T&& k )const { return get()[ std::forward<T>(k)]; }

         template<typename S>
         friend S& operator << ( S& stream, const member_proxy& member) {
             return stream << member.get();
         }

         template<typename R>
         auto operator=( R&& r ) {
             get() = std::forward<R>(r);
             return *this;
         }

         template<typename R>
         operator R ()const { return get(); }

         /*
         operator decltype( ((ProxyObject*)nullptr)->template get<I,Name,mptr>())()
                 { return get(); }
         operator decltype( ((const ProxyObject*)nullptr)->template get<I,Name,mptr>()) ()const
                 { return get(); }
                 */
    };

} /// clio
