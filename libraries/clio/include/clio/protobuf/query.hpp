#pragma once
#include <clio/protobuf/any.hpp>
#include <functional>

namespace clio {

    namespace protobuf {

        struct field_query;
        struct variant_query;

        struct variant_query {
            varuint32                  type;
            std::vector<field_query>   fields;
        };

        struct query {
            std::vector<field_query>     fields; 
            std::vector<variant_query>   variant_fields; /// only apply these fields when the type has them
        };

        struct field_query {
            varuint32  number;
            bytes      args;  /// args as protobuf encoded tuple
            query      filter; /// apply to result 
        };

        CLIO_REFLECT( variant_query, type, fields )
        CLIO_REFLECT( field_query, number, args, filter )
        CLIO_REFLECT( query, fields, variant_fields )



        template<typename T>
        any dispatch( T&& obj, const query& q ) {
            any result;
            result.members.reserve( q.fields.size() );

            for( const auto& field : q.fields ) {
                reflect<T>::get( int64_t(field.number), [&]( auto mptr ){
                    using member_ptr_type = decltype(mptr);
                    if constexpr ( std::is_member_function_pointer_v<member_ptr_type> ) {
                        using return_type = decltype( result_of(mptr) ); 
                        using param_type = decltype( args_as_tuple(mptr) ); 
                //        std::cerr<< boost::core::demangle(typeid(param_type).name()) <<"\n";

                        param_type params;
                        input_stream in( field.args.data.data(), field.args.data.size() ); 
                        (void) from_protobuf_object( params, in );
                        std::apply( [&]( auto... args ){
                            if constexpr( is_std_variant<return_type>::value ) {
                                /// TODO:
                            } else if constexpr( not reflect<return_type>::is_struct ) 
                                result.add( field.number, (obj.*mptr)( args... ) );
                            else {
                                any field_data  = dispatch( (obj.*mptr)( args...) , field.filter );
                                result.add( field.number, clio::to_bin( field_data ) );
                            }
                        }, params ); 

                    } else if constexpr ( not std::is_member_function_pointer_v<member_ptr_type> ) {
                        using member_type = std::decay_t<decltype( obj.*mptr )>;

                        if constexpr( is_std_tuple<member_type>::value ) {
                          /// TODO:
                        } 
                        else if constexpr( is_std_variant<member_type>::value ) {
                          /// TODO:
                        } 
                        else if constexpr ( is_std_vector<member_type>::value ) 
                        {
                            using value_type = typename is_std_vector<member_type>::value_type;
                            if constexpr( is_std_variant<value_type>::value ) {
                                /// TODO:
                            } else if constexpr ( std::is_arithmetic_v< value_type > ) { 
                                result.add( field.number, obj.*mptr ); 
                            } else if constexpr( reflect<value_type>::is_struct ) {
                                for( const auto& item : obj.*mptr ) {
                                    any field_data  = dispatch( item, field.filter );
                                    result.add( field.number, clio::to_bin( field_data ) );
                                }
                            } else {
                                for( const auto& item : obj.*mptr ) {
                                    result.add( field.number, item ); 
                                }
                            }
                        }
                        else if constexpr ( not reflect<member_type>::is_struct ) 
                        {
                            result.add( field.number, obj.*mptr );
                        }
                        else
                        {
                            any field_data  = dispatch( obj.*mptr, field.filter );
                            result.add( field.number, clio::to_bin( field_data ) );
                        }

                    }
                });
            }
            return result;
        }

       struct query_proxy {
           protobuf::query q;

           template<uint32_t idx, uint64_t Name, auto MemberPtr, typename... Args>
           query_proxy& call(  Args&&... args ) {
               return *this;
           }
           template<uint32_t idx, uint64_t Name, auto MemberPtr>
           void  get() {
           }

           template<uint32_t idx, uint64_t Name, auto MemberPtr>
           void  get()const {
           }

           template<typename Mptr, typename Filter>
           query_proxy& call( const meta& ref, Mptr mptr, Filter&& filter ) {
               using result_type = decltype( result_of(mptr) );
               using args_tuple  = decltype(args_as_tuple( mptr ));

               if constexpr ( std::tuple_size<args_tuple>::value == 1 ) {
                    auto params = args_tuple( std::forward<Filter>(filter) );
                    q.fields.push_back( {
                        .number = ref.number,
                        .args   = to_protobuf( params ),
                    });
               } else {
                    typename reflect<result_type>::template proxy<query_proxy> rp;
                    filter( rp );
                    q.fields.push_back( {
                        .number = ref.number,
                        .filter = std::move(rp->q)
                    });
               }
               return *this;
           }

           template<typename Mptr>
           query_proxy& call( const meta& ref, Mptr mptr ) {
               q.fields.push_back( {
                   .number = ref.number,
               });
               return *this;
           }


           template<typename Mptr, typename Filter, typename... Args>
           query_proxy& call( const meta& ref, Mptr mptr, Filter&& filter, Args&&... args ) {
               using result_type = decltype( result_of(mptr) );
               using args_tuple  = decltype(args_as_tuple( mptr ));
               /** test to see whether the user passed a filter by comparing the number of args,
                *  if the number of var args equals the number of paramters then a filter was passed,
                *  otherwise the Filter is being interpreted as the first argument. This means that in
                *  order to use a filter you must pass all args, if you don't want a filter on the result
                *  then you can skip trailing args.
                */
               if constexpr ( std::tuple_size<args_tuple>::value == sizeof...(Args) ) {
                    auto params = args_tuple( std::forward<Args>(args)... );

                    if constexpr( reflect<result_type>::is_struct ) {
                        typename reflect<result_type>::template proxy<query_proxy> rp;
                        filter( rp );
                        q.fields.push_back( {
                            .number = ref.number,
                            .args   = to_protobuf( params ),
                            .filter = std::move(rp->q)
                        });
                    } else {
                        q.fields.push_back( {
                            .number = ref.number,
                            .args   = to_protobuf( params ),
                        });
                    }
               } else {
                    auto params = args_tuple( std::forward<Filter>(filter), std::forward<Args>(args)... );

                    if constexpr( reflect<result_type>::is_struct ) {
                        typename reflect<result_type>::template proxy<query_proxy> rp;
                        q.fields.push_back( {
                            .number = ref.number,
                            .args   = to_protobuf( params ),
                            .filter = std::move(rp->q)
                        });
                    } else {
                        q.fields.push_back( {
                            .number = ref.number,
                            .args   = to_protobuf( params ),
                        });
                    }
               }
               return *this;
           }
       };


    } // namespace protobuf

} /// namespace clio
