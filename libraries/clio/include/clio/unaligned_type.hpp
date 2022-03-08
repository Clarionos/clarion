#pragma once

namespace clio {
 
    #define FRACPACK __attribute__((packed,aligned(1)))

    /**
     *  Reading from a char* is potentially unaligned, so we always cast
     *  through this type to warn the compiler that unaligned_type::val 
     *  might be unaligned.
     */
    template<typename T>
    struct FRACPACK unaligned_type {
       T val;
    
       unaligned_type() = default;
    
       template<typename C>
       unaligned_type( C&& c ):val( std::forward<C>(c) ){}
    
       /** we must return by copy because if we return by
        *  reference the caller won't have the context to know that
        *  the pointer may be unaligned.
        *
        *  WARNING: do not change to return a reference 
        */
       operator T()const { return val; }
       unaligned_type& operator = ( const T& v ) { val = v; return *this;}
    };

} /// namespace clio
