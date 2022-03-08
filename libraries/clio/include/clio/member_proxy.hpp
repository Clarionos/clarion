#pragma once

namespace clio {


    /**
     *  This works just like member_proxy, except instead of using "undefined behavior" to grab
     *  a pointer based upon an assumed memory layout and potentially aliasing pointers, this
     *  member proxy takes the proxy in the constructor.
     */
    template<uint32_t I, uint64_t Name, auto mptr, typename ProxyObject>
    struct member_proxy {
        private:
         ProxyObject& _proxy;

        public:
         member_proxy( ProxyObject& o ):_proxy(o){}


         /**
          *  This object is created on a type created by the macro, I represents the member number in the
          *  containing type. The containing type's first member is a ProxyObject. 
          *
          *  
          *  safe_flat_view<T> => reflect<T>::member_proxy<proxy_impl>
          *
          *  struct proxy<proxy_impl>  {
          *     proxy_impl _clio_proxy_obj;
          *     member_proxy<0, ptr, proxy_impl> member_zero() { return {_clio_proxy_obj}; }
          *     member_proxy<1, ptr, proxy_impl> member_one(){_clio_proxy_obj}; }
          *     member_proxy<2, ptr, proxy_impl> member_two(){_clio_proxy_obj}; }
          *  }
          *  a packed flat buffer.  
          *
          *  Let char* buf = point to a flat buffer;
          *  Let reinterpet buf as memper_proxy<proxy_impl>*, this makes the address of proxy__ equal to
          *  the address of member_proxy<T> because it is the first element. 
          *
          *  because member_proxy has no values it takes 1 byte in member_proxy and the value of that byte
          *  is never read by member_proxy... member_proxy always gets the address of _clio_proxy_obj and then
          *  does offset math.
          *
          */
         /*
         constexpr auto proxyptr()const { 
             return _proxy;
         }
         constexpr auto proxyptr(){ 
             return _proxy;
         }
         */
         constexpr auto& get()const { return *(_proxy.template get<I,Name,mptr>()); }
         constexpr auto& get()      { return *(_proxy.template get<I,Name,mptr>()); }

         template<typename... Ts>
         constexpr auto operator()(Ts&&... args) { 
             return _proxy.template call<I,Name,mptr>( std::forward<Ts>(args)...); 
         }
         template<typename... Ts>
         constexpr auto operator()(Ts&&... args)const { 
             return _proxy.template call<I,Name,mptr>( std::forward<Ts>(args)...); 
         }

         constexpr auto* operator->(){ return (_proxy.template get<I,Name,mptr>()); }
         constexpr const auto* operator->()const { return (_proxy.template get<I,Name,mptr>()); }

         constexpr auto& operator*(){ return get(); }
         constexpr const auto& operator*()const { return get(); }

         constexpr auto& mget(){ return get(); }

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

         bool valid()const { return _proxy.template get<I,Name,mptr>() != nullptr; }
         bool valid()      { return _proxy.template get<I,Name,mptr>() != nullptr; }

    };

} /// clio
