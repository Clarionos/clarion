#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_args_sizes_get(__wasi_size_t* argc, __wasi_size_t* argv_buf_size)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("args_sizes_get")))
{
   static_assert(sizeof(__wasi_size_t) == sizeof(uint32_t));
   [[clang::import_module("clarion"), clang::import_name("get_arg_counts")]] void get_arg_counts(
       __wasi_size_t * argc, __wasi_size_t * argv_buf_size);
   get_arg_counts(argc, argv_buf_size);
   return 0;
}
