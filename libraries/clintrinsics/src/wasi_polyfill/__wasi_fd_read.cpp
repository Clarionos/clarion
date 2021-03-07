#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_fd_read(
    __wasi_fd_t fd,
    const __wasi_iovec_t *iovs,
    size_t iovs_len,
    __wasi_size_t *nread) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                         __import_name__("fd_read")))
{
    clintrinsics::fatal("__wasi_fd_read not implemented");
}
