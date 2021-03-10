#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_fd_close(
    __wasi_fd_t fd) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                   __import_name__("fd_close")))
{
    clintrinsics::fatal("__wasi_fd_close not implemented");
}
