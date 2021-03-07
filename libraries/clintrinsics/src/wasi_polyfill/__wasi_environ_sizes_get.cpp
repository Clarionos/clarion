#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_environ_sizes_get(
    __wasi_size_t *environc,
    __wasi_size_t *environ_buf_size) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                                    __import_name__("environ_sizes_get")))
{
    clintrinsics::fatal("__wasi_environ_sizes_get not implemented");
}
