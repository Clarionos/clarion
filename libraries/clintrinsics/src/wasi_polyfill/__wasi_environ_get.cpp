#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_environ_get(
    uint8_t **environ,
    uint8_t *environ_buf) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                         __import_name__("environ_get")))
{
    clintrinsics::fatal("__wasi_environ_get not implemented");
}
