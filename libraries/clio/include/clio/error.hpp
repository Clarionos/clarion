#pragma once

namespace clio {
    /**
     *  On some platforms we need to disable
     *  exceptions and do something else, this
     *  wraps that.
     */
    template<typename T>
    [[noreturn]] void throw_error( T&& e ){
        #ifdef __cpp_exceptions
            throw e;
        #else
            abort();
        #endif
    }
};
