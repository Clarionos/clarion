#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_fd_prestat_get(__wasi_fd_t fd, __wasi_prestat_t* buf)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_prestat_get")))
{
   return __WASI_ERRNO_BADF;
}
