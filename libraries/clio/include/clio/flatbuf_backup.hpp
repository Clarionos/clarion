#pragma once
#include <clio/error.hpp>
#include <clio/stream.hpp>
#include <clio/name.hpp>

namespace clio {

    struct flat_view_proxy_impl;

    template<typename T>
    class flat_ptr;

    template<typename>
    struct is_flat_ptr : std::false_type {};
    
    template<typename T>
    struct is_flat_ptr<flat_ptr<T>> : std::true_type { using value_type = T; };

    template<typename T>
    class flat {
        using T::flat_type_not_defined;
    };

    template<>
    class flat<std::string>;


    template<typename... Ts>
    class flat<std::variant<Ts...>>;

    template<typename T>
    auto get_view_type() {
        if constexpr( is_flat_ptr<T>::value ) {
            return get_view_type<typename T::value_type>();
        } else if constexpr( reflect<T>::is_struct )  {
            using view_type = typename reflect<T>::template proxy<flat_view_proxy_impl>;
            return (view_type*)(nullptr);
        }
        else if constexpr( std::is_trivially_copyable<T>::value ) 
            return (T*)(nullptr);
        else
            return (flat<T>*)nullptr; 
    }

    template<typename T>
    using flat_view = std::remove_pointer_t<decltype(get_view_type<T>())>;

    struct offset_ptr {
        uint32_t offset;

        template<typename T>
        auto* get()const;
    };

    template<typename T> 
    constexpr bool contains_offset_ptr();

    template<typename T>
    struct tuple_contains_offset;


    template<typename First, typename...Ts>
    constexpr uint32_t get_contains_offset_ptr()  {
        if constexpr( sizeof...(Ts) == 0 )
            return contains_offset_ptr<First>();
        else {
            return contains_offset_ptr<First>() | 
                   get_contains_offset_ptr<Ts...>();
        }
    }

    template<typename... Ts>
    struct tuple_contains_offset< std::tuple<Ts...> > {
        static constexpr const auto value = get_contains_offset_ptr<Ts...>();
    };
   

    /** 
     *  Recursively checks the types for any field which requires dynamic allocation,
     */
    template<typename T> 
    constexpr bool contains_offset_ptr() {
        if constexpr ( is_flat_ptr<T>::value ) {
           return true;
        } else if constexpr( is_std_tuple<T>::value ) {
           return tuple_contains_offset<T>::value;           
        } else if constexpr( is_std_variant<T>::value ) {
           return contains_offset_ptr<typename is_std_variant<T>::alts_as_tuple>();  
        } else if constexpr( std::is_same_v<std::string,T> ) {
            return true;
        } else if constexpr( is_std_vector<T>::value ) {
            return true;
        } else if constexpr( clio::reflect<T>::is_struct ) {
            bool is_flat = true;
            clio::reflect<T>::for_each( [&]( const clio::meta& ref, auto mptr ){
                 using member_type = std::decay_t<decltype(clio::result_of_member(mptr))>;
                 is_flat &= not contains_offset_ptr<member_type>();
            });
            return not is_flat;
        } else if constexpr( std::is_arithmetic_v<T> ) {
            return false;
        } else if constexpr( std::is_trivially_copyable<T>::value ) {
            return false;
        } else {
            T::contains_offset_ptr_not_defined;
        }
    }


    template<typename T>
    constexpr uint32_t flatpack_size() {
        if constexpr( is_std_variant<T>::value ) {
            return 16;
        } else if constexpr( reflect<T>::is_struct ) {
            uint32_t size = 0;
            reflect<T>::for_each( [&]( const meta& ref, const auto& mptr ){
                 using member_type = decltype(result_of_member(mptr));
                 if constexpr ( contains_offset_ptr<member_type>() ) {
                     size += sizeof(offset_ptr);
                 } else {
                     size += flatpack_size<member_type>();
                 }
            });
            return size;
        } else if constexpr( std::is_same_v<std::string,T> || is_std_vector<T>::value) {
            return sizeof(offset_ptr);
        } else if constexpr( std::is_arithmetic_v<T> ) {
            return sizeof(T);
        } else if constexpr( std::is_trivially_copyable<T>::value ) {
            return sizeof(T);
        } else {
            T::flatpack_size_not_defined;
        }
    }

    template<uint32_t I, typename Tuple>
    struct get_tuple_offset;

    template<uint32_t Idx, typename First, typename...Ts>
    constexpr uint32_t get_offset()  {
        static_assert( Idx < sizeof...(Ts) + 1, "index out of range" );
        if constexpr ( Idx == 0 ) return 0;
        else if constexpr( sizeof...(Ts) == 0 )
            return flatpack_size<First>();
       //     return get_flat_size( ((const First*)(nullptr)) );
        else {
            return get_offset<Idx-1, Ts...>() + flatpack_size<First>();
       //     return get_offset< Idx-1, Ts...>() + get_flat_size( ((const First*)(nullptr)) );
        }
    }

    template<uint32_t I, typename... Args>
    struct get_tuple_offset< I, std::tuple<Args...> > {
        static constexpr const uint32_t value = get_offset<I, Args...>(); 
    };


    struct flat_view_proxy_impl {

        /** This method is called by the reflection library to get the field */
        template<uint32_t idx, uint64_t Name, auto MemberPtr>
        constexpr auto* get() {
            using class_type = decltype( clio::class_of_member(MemberPtr) );
            using tuple_type = typename clio::reflect<class_type>::struct_tuple_type;
            using member_type = decltype( clio::result_of_member(MemberPtr) );

            constexpr uint32_t offset = clio::get_tuple_offset<idx,tuple_type>::value;

            char* out_ptr = reinterpret_cast<char*>(this)+offset;

            if constexpr ( contains_offset_ptr<member_type>() ) {
                clio::offset_ptr* ptr = reinterpret_cast<clio::offset_ptr*>(out_ptr);
                return ptr->get<member_type>(); 
            } else  {
                return reinterpret_cast<member_type*>(out_ptr);
            }
        }

        template<uint32_t idx, uint64_t Name, auto MemberPtr>
        constexpr const auto* get()const {
            using class_type = decltype( clio::class_of_member(MemberPtr) );
            using tuple_type = typename clio::reflect<class_type>::struct_tuple_type;
            using member_type = decltype( clio::result_of_member(MemberPtr) );

            constexpr uint32_t offset = clio::get_tuple_offset<idx,tuple_type>::value;

            auto out_ptr = reinterpret_cast<const char*>(this)+offset;

            if constexpr ( contains_offset_ptr<member_type>() ) {
                const clio::offset_ptr* ptr = reinterpret_cast<const clio::offset_ptr*>(out_ptr);
                return ptr->get<member_type>(); 
            } else  {
                return reinterpret_cast<const member_type*>(out_ptr);
            }
        }

    };

    template<>
    class flat<std::string> {
        public:
            uint32_t       size()const  { return _size; }
            const char*    c_str()const { return _size ? _data : (const char*)this; }
            const char*    data()const  { return _size ? _data : nullptr; }
            char*          data()       { return _size ? _data : nullptr; }

            operator std::string_view()const { return std::string_view( _data, _size ); }

            template<typename S>
            friend S& operator << ( S& stream, const flat<std::string>& str ) {
                return stream << str.c_str();
            }
        private:
            uint32_t _size = 0;
            char     _data[];
    };

    template<typename... Ts>
    class flat<std::variant<Ts...>> {
        public:
            uint64_t       type = 0;
            uint32_t       flat_data = 0;
            offset_ptr     offset_data;

            flat() {
                static_assert( sizeof(flat) == 16 );
                static_assert( std::is_trivially_copyable< flat >::value );
            }

            int64_t index_from_type()const {
                return get_index_from_type<Ts...>();
            }

            void init_variant( std::variant<Ts...>& v ) {
                _init_variant<Ts...>( v );
            }

            template<typename Visitor>
            void visit( Visitor&& v ) {
                _visit_variant<Visitor,Ts...>( std::forward<Visitor>(v) );
            }
        private:
            template<typename First, typename... Rest> 
            int64_t get_index_from_type()const {
                if constexpr( sizeof...(Rest) == 0 ) {
                    return get_type_hashname<First>() != type;
                } else {
                    if( get_type_hashname<First>() == type ) 
                        return 0;
                    else return 1 + get_index_from_type<Ts...>();
                }
            }

            template<typename First, typename... Rest> 
            void _init_variant( std::variant<Ts...>& v )const {
                if( get_type_hashname<First>() == type )
                    v = First();
                else if constexpr( sizeof...(Rest) > 0 ) {
                    _init_variant<Rest...>( v );
                }
            }
            template<typename Visitor, typename First, typename... Rest> 
            void _visit_variant( Visitor&& v )const {
                if( get_type_hashname<First>() == type ) {
                    if constexpr ( not get_contains_offset_ptr<First>() and flatpack_size<First>() <= 8 ) {
                        v( *((const flat_view<First>*)&flat_data) );
                    } else {
                        v( *((const flat_view<First>*) ( ((char*)(&offset_data)) + offset_data.offset ) ) );
                    }
                }
                else if constexpr( sizeof...(Rest) > 0 ) {
                    _visit_variant<Visitor, Rest...>( std::forward<Visitor>(v) );
                }
            }
    };


    /// T == value of the array elements
    template<typename T>
    class flat<std::vector<T>> {
        public:
            auto& operator[]( uint32_t index ) {
                if( index >= _size )
                    throw_error( stream_error::overrun );
                /** in this case the data is a series of offset_ptr<> */
                if constexpr( std::is_same<T,std::string>::value ) {
                    auto ptr_array =  reinterpret_cast<offset_ptr*>(_data);
                    return *reinterpret_cast<flat<std::string>*>( ptr_array[index].get< std::vector<T> >() );
                } else if constexpr( contains_offset_ptr<T>() ) {
                    auto ptr_array =  reinterpret_cast<offset_ptr*>(_data);
                    return *reinterpret_cast<flat_view<T>*>( ptr_array[index].get< std::vector<T> >() );
                } else if constexpr ( reflect<T>::is_struct ) { /// the data is a series of packed T
                    const auto offset = index * flatpack_size<T>(); 
                    return *reinterpret_cast<flat_view<T>*>( &_data[offset] );
                } else if constexpr ( std::is_trivially_copyable<T>::value ) {
                    auto T_array =  reinterpret_cast<T*>(_data);
                    return T_array[index];
                } else {
                    T::is_not_a_known_flat_type;
                }
            }
            const auto& operator[]( uint32_t index )const {
                if( index >= _size )
                    throw_error( stream_error::overrun );

                /** in this case the data is a series of offset_ptr<> */
                if constexpr( std::is_same<T,std::string>::value ) {
                    auto ptr_array =  reinterpret_cast<offset_ptr*>(_data);
                    return *reinterpret_cast<const flat<std::string>*>( ptr_array[index].get< std::vector<T> >() );
                }else if constexpr( contains_offset_ptr<T>() ) {
                    auto ptr_array =  reinterpret_cast<const offset_ptr*>(_data+sizeof(offset_ptr)*index);
                    const auto& r = *reinterpret_cast<const flat_view<T>*>( ptr_array->get< std::vector<T> >() );
                    return r; 
                } else if constexpr ( reflect<T>::is_struct ) { /// the data is a series of packed T
                    const auto offset = index * flatpack_size<T>(); 
                    return *reinterpret_cast<const flat_view<T>*>( &_data[offset] );
                } else if constexpr ( std::is_trivially_copyable<T>::value ) {
                    auto T_array =  reinterpret_cast<const T*>(_data);
                    return T_array[index];
                } else {
                    T::is_not_a_known_flat_type;
                }
            }


            uint32_t size()const { return _size; }

        private:
            uint32_t _size = 0;
            char     _data[];
    };

    template<typename T>
    auto* offset_ptr::get()const {
        const auto ptr = ((char*)this)+offset;
        if constexpr( is_flat_ptr<T>::value ) {
            return reinterpret_cast< decltype(get_view_type<T::value_type>()) >(ptr+4);
        } else if constexpr( reflect<T>::is_struct ) {
            return reinterpret_cast<flat_view<T>*>(ptr);
        } else if constexpr( std::is_same_v<std::string,T> ) {
            return reinterpret_cast< flat<std::string>* >(ptr);
        } else if constexpr( is_std_vector<T>::value ) {
            return reinterpret_cast< flat<T>* >(ptr);
        } else {
            T::is_not_reflected_for_offset_ptr;
        }
    }


    /**
     * Verifies that the type pointed at could be unpacked without a buffer overflow,
     * which means that all offset pointers are in bounds. It does this by unpacking
     * the stream without allocating any memory.
     */
    struct validate_input_stream : public input_stream {
        using input_stream::input_stream;
    };

    template<typename T, typename InputStream>
    void flatcheck( InputStream& stream ) {
       if constexpr ( is_flat_ptr<T>::value ) {
       } else if constexpr ( is_std_variant<T>::value ) {
            flat<T> fv;
            stream.read( &fv, sizeof(fv)  );

            T temp; /// this could do memory allocation for certain types... but hopefully won't,
                    /// this is used to do std::visit in the next line... in theory we could do
                    /// a dispatcher that only deals in types and not values.
            fv.init_variant( temp ); 
            std::visit( [&]( auto& iv ){
               using item_type = std::decay_t< decltype(iv) >;
               if constexpr ( get_contains_offset_ptr<item_type>() ) {
                  /// the stream.pos is at the END of reading the variant, which
                  /// should be the same as the end of flat<variant>::offset_data
                 InputStream in( stream.pos + fv.offset_data.offset-sizeof(offset_ptr), stream.end );
                 flatunpack( iv, in );
               } else if constexpr ( flatpack_size<item_type>() <= 8 ){
               } else {
                 InputStream in( stream.pos + fv.offset_data.offset-sizeof(offset_ptr), stream.end );
                 flatunpack( iv, in );
               }
            }, temp);
            /// maybe deref pointer..
       } else if constexpr ( std::is_same_v<T,std::string> ) {
            uint32_t size; 
            stream.read( &size, sizeof(size) );
            stream.skip( size );
       } else if constexpr ( is_std_vector<T>::value ) {
            uint32_t size; 
            stream.read( &size, sizeof(size) );
            if constexpr( contains_offset_ptr< typename is_std_vector<T>::value_type >() ) {
                auto start = stream.pos;
                stream.skip( size * sizeof( offset_ptr ) );

                offset_ptr* ptr = (offset_ptr*)(start);
                for( uint32_t i = 0; i < size; ++i ) {
                    InputStream in( ((char*)ptr) + ptr->offset, stream.end );
                    flatcheck<typename T::value_type>(in);
                }
            } else {
                stream.skip( size * flatpack_size<typename T::value_type>() );
            }
       } else if constexpr ( reflect<T>::is_struct ) {
            if constexpr( contains_offset_ptr< T >() ) {
                reflect<T>::for_each( [&]( const meta& ref, const auto& mptr ){
                    using member_type = decltype(result_of_member(mptr));

                    if constexpr ( contains_offset_ptr<member_type>() ) {
                        offset_ptr ptr;
                        stream.read(&ptr,sizeof(ptr));

                        InputStream substream( stream.pos+ptr.offset-sizeof(ptr), stream.end );
                        flatcheck<member_type>( substream );
                    } else {
                        stream.skip( flatpack_size<member_type>() );
                    }
                });
            } else {
                stream.skip( flatpack_size<typename T::value_type>() );
            }
       } else if constexpr( std::is_trivially_copyable<T>::value ) {
            stream.skip( sizeof(T) );
       } else {
            T::is_not_defined;
       }
    }

    template<typename T, typename S>
    void flatunpack( T& v, S& stream ) {
       if constexpr ( is_flat_ptr<T>::value ) {
            uint32_t size; 
            stream.read( &size, sizeof(size) );
            v.reset(size);
            stream.read( v.data(), size ) ;
       } else if constexpr ( is_std_variant<T>::value ) {
           flat<T> fv;
           stream.read( &fv, sizeof(fv)  );
           fv.init_variant( v ); 
           std::visit( [&]( auto& iv ){
              using item_type = std::decay_t< decltype(iv) >;
              if constexpr ( get_contains_offset_ptr<item_type>() ) {
                 /// the stream.pos is at the END of reading the variant, which
                 /// should be the same as the end of flat<variant>::offset_data
                input_stream in( stream.pos + fv.offset_data.offset-sizeof(offset_ptr), stream.end );
                flatunpack( iv, in );
              } else if constexpr ( flatpack_size<item_type>() <= 8 ){
                input_stream st( (const char*)&fv.flat_data, 8 );
                flatunpack( iv, st );
              } else {
                input_stream in( stream.pos + fv.offset_data.offset-sizeof(offset_ptr), stream.end );
                flatunpack( iv, in );
              }
           }, v);
       } else if constexpr ( std::is_same_v<T,std::string> ) {
            uint32_t size; 
            stream.read( &size, sizeof(size) );
            v.resize(size);
            stream.read( v.data(), size ) ;
            stream.skip(1); // null
       } else if constexpr ( is_std_vector<T>::value ) {
            uint32_t size; 
            stream.read( &size, sizeof(size) );
            v.resize( size );

            if constexpr( contains_offset_ptr< typename is_std_vector<T>::value_type >() ) {
                for( auto& item : v ) {
                    offset_ptr ptr;
                    stream.read(&ptr,sizeof(ptr));

                    /// TODO: we don't know the size of the buffer here...is it safe?
                    input_stream in( stream.pos + ptr.offset-sizeof(ptr), stream.end );
                    flatunpack( item, in );
                }
            } else {
                for( auto& item : v ) {
                    flatunpack( item, stream );
                }
            }
        } else if constexpr ( reflect<T>::is_struct ) {
            reflect<T>::for_each( [&]( const meta& ref, const auto& mptr ){
                auto& member = v.*mptr;
                using member_type = decltype(result_of_member(mptr));

                if constexpr ( contains_offset_ptr<member_type>() ) {
                    offset_ptr ptr;
                    stream.read(&ptr,sizeof(ptr));

                    input_stream substream( stream.pos+ptr.offset-sizeof(ptr), stream.end );
                    flatunpack( member, substream );
                } else {
                    flatunpack( member, stream );
                }
            });
        } else if constexpr( std::is_trivially_copyable<T>::value ) {
            stream.read( (char*)&v, sizeof(v) );
        } else {
            T::is_not_defined;
        }
    }

    template<typename T, typename S>
    uint32_t flatpack( const T& v, S& stream ) {
        uint32_t alloc_pos = 0; 
        uint32_t cur_pos  = 0;

        if constexpr ( is_flat_ptr<T>::value ) {
            uint32_t size = v.size();
            stream.write( &size, sizeof(size) );
            cur_pos += sizeof(size);

            if( size ) {
                stream.write( v.data(), v.size() );
            }
        } else if constexpr ( is_std_variant<T>::value ) {
            alloc_pos = flatpack_size<T>();
            std::visit( [&]( const auto& iv ){
                using item_type = std::decay_t< decltype(iv) >;
                flat<T> fv;
                fv.type = get_type_hashname<item_type>();
                if constexpr ( not get_contains_offset_ptr<item_type>() and flatpack_size<item_type>() <= 8 ) {
                    fixed_buf_stream st( (char*)&fv.flat_data, 8 );
                    flatpack( iv, st );

                    static_assert( sizeof(fv) == 16 );
                    stream.write( &fv, sizeof(fv)  );
                } else {
                    fv.offset_data.offset = (alloc_pos - (cur_pos+12)); //+8 because we haven't written the type yet / pad
                    size_stream size_str;
                    flatpack( iv, size_str );
                    alloc_pos += size_str.size; 

                    static_assert( sizeof(fv) == 16 );
                    stream.write( &fv, sizeof(fv)  );

                    if constexpr( std::is_same_v<size_stream,S> ) {
                         stream.skip( size_str.size ); /// we already calculated this above
                    }
                    else {
                        /// now pack the member into the allocated spot
                        fixed_buf_stream substream( stream.pos+fv.offset_data.offset-sizeof(fv.offset_data), 
                                                    size_str.size );
                        if( substream.end > stream.end )
                            throw_error( stream_error::overrun );
                        flatpack( iv, substream );
                    }
                }
            }, v );
        } else if constexpr ( std::is_same_v<std::string,T> ) {
                uint32_t size = v.size();
                stream.write( &size, sizeof(size) );
                cur_pos += sizeof(size);

                if( size ) {
                    stream.write( v.data(), v.size() );
                    stream.write( "\0", 1 ); /// null term
                    cur_pos += v.size() + 1;
                }
        } else if constexpr ( is_std_vector<T>::value ) {

            if constexpr( contains_offset_ptr< typename T::value_type >() ) {

                uint32_t size = v.size();
                stream.write( &size, sizeof(size) );
                cur_pos += sizeof(size);
                alloc_pos += sizeof(size) + size * sizeof(offset_ptr);


                for( const auto& member: v ) {

                   if constexpr ( std::is_same_v<std::string,typename T::value_type> || is_std_vector<typename T::value_type>::value ) {
                       if( member.size() == 0 ) {
                            offset_ptr ptr = { .offset = 0 };
                            stream.write( &ptr, sizeof(ptr) );
                            cur_pos += sizeof(ptr);
                            continue;
                       }
                   }
                    
                    size_stream size_str;
                    flatpack( member, size_str );

                    offset_ptr ptr = { .offset = (alloc_pos - cur_pos) };
                    if( ptr.offset == 0 ) exit(-1);

                   // std::cout << "   vector element has size: " << size_str.size << "  and offset: " << ptr.offset <<"\n";

                    alloc_pos += size_str.size; 

                    stream.write( &ptr, sizeof(ptr) );
                    //std::cout << "pack element off: " << ptr.offset <<" " <<int64_t(stream.pos)<<"\n";

                    cur_pos += sizeof(ptr);

                    //? cur_pos += size_str.size;
                    if constexpr( std::is_same_v<size_stream,S> ) {
                         stream.skip( size_str.size ); /// we already calculated this above
                    }
                    else {
                        /// now pack the member into the allocated spot
                        fixed_buf_stream substream( stream.pos+ptr.offset-sizeof(ptr), 
                                                    size_str.size ); //ptr.size );
                        if( substream.end > stream.end )
                            throw_error( stream_error::overrun );
                        flatpack( member, substream );
                    }
                }
            }
            else {
               //  std::cout << "vector type, T, is flat types:  " << boost::core::demangle(typeid(typename T::value_type).name()) <<"\n";
                uint32_t size = v.size();
                stream.write( &size, sizeof(size) );
                cur_pos += sizeof(size);
                for( const auto& item : v ) {
                    cur_pos += flatpack( item, stream );
                }
            }
        } else if constexpr ( reflect<T>::is_struct ) {
            alloc_pos = flatpack_size<T>();//<typename reflect<T>::struct_tuple_type>::value;
            reflect<T>::for_each( [&]( const meta& ref, const auto& mptr ){

                const auto& member = v.*mptr;

                 /*if constexpr( not std::is_same_v<size_stream,S> ) {
                    std::cerr << "member: " << ref.name 
                             <<" stream pos: " << int64_t(stream.pos - stream.begin)
                             <<"\n";
                 }
                 */

                using member_type = decltype(result_of_member(mptr));
                if constexpr ( contains_offset_ptr<member_type>() ) {
                    
                    /* if constexpr( not std::is_same_v<size_stream,S> ) {
                        std::cerr<< ref.name << " contains ptr, so we need to pack it\n";
                     }
                     */

                    if constexpr ( std::is_same_v<std::string,member_type> || is_std_vector<member_type>::value ) {
                       if( member.size() == 0 ) {
                            offset_ptr ptr = { .offset = 0 };
                            stream.write( &ptr, sizeof(ptr) );
                            cur_pos += sizeof(ptr);

                      /*       if constexpr( not std::is_same_v<size_stream,S> ) {
                                std::cerr<<"optimized empty object: " << ref.name << "\n";
                             }
                             */

                            return;
                       }
                    } 

                    size_stream size_str;
                    flatpack( member, size_str );

                 


                    offset_ptr ptr = { .offset = alloc_pos - cur_pos };

                    /*
                 if constexpr( not std::is_same_v<size_stream,S> ) {
                    std::cerr<<"sub object size calculted to be: " <<size_str.size<<"   with offset: " << ptr.offset<< "\n";
                 }   
                 */

                    if( ptr.offset == 0 ) exit(-1);


                    alloc_pos += size_str.size; 
                    stream.write( &ptr, sizeof(ptr) );
                    cur_pos += sizeof(ptr);


                    if constexpr( std::is_same_v<size_stream,S> ) {
                         stream.skip( size_str.size ); /// we already calculated this above
                    }
                    else {
                        /// now pack the member into the allocated spot
                        fixed_buf_stream substream( stream.pos+ptr.offset-sizeof(ptr), size_str.size );

                        if( substream.end > stream.end ) {
                            throw_error( stream_error::overrun );
                        }
                        flatpack( member, substream );
                    }
                } else {
                    cur_pos += flatpack( member, stream );
                }
            });
        } else if constexpr( std::is_trivially_copyable<T>::value ) {
            stream.write( &v, sizeof(v) );
            cur_pos  += sizeof(v);
        } else {
            T::flatpack_is_not_defined;
        }
        return cur_pos;
    }


    /**
     * Behaves like a shared_ptr<T>, copies will all point to the same flat data array.
     */
    template<typename T>
    class flat_ptr {
        public:
            typedef T value_type;

            flat_ptr( const T& from ) {
                    clio::size_stream ss;
                    clio::flatpack( from, ss );
                    _size = ss.size;
                    _data = std::make_shared<char>(_size); ///< c++20 
                    // c++17 _data = std::shared_ptr<char>( new char[_size], [](char* c){ delete[] c; } );
                    fixed_buf_stream  out( _data.get(), _size ); 
                    clio::flatpack( from, out );
            }
            flat_ptr() {};
            operator bool()const { return _size; };

            auto  operator*()      { 
                return *reinterpret_cast<flat_view<T>*>(_data.get()); 
            }
            auto  operator*()const { 
                return *reinterpret_cast<const flat_view<T>*>(_data.get()); 
            }

            auto  operator->()      { 
                return reinterpret_cast<flat_view<T>*>(_data.get()); 
            }
            auto  operator->()const { 
                return reinterpret_cast<const flat_view<T>*>(_data.get()); 
            }
            const char* data()const  { return _data.get(); }
            char*       data()       { return _data.get(); }
            size_t      size()const  { return _size;       }

            void reset() {
                _data.reset();
                _size = 0;
            }

            void reset( size_t s ) {
                _size = s;
                _data = std::make_shared<char>(_size); ///< c++20 
            }

            operator T()const {
                T tmp;
                input_stream in(_data.get(), _size );
                clio::flatunpack( tmp, in );
                return tmp;
            }

        private:
            std::shared_ptr<char> _data;
            size_t                _size = 0;
    };

} /// namespace clio
