#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_args_get(
    uint8_t **argv,
    uint8_t *argv_buf) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                      __import_name__("args_get")))
{
    [[clang::import_module("clarion"), clang::import_name("get_args")]] void get_args(
        uint8_t * *argv, uint8_t * argv_buf);
    get_args(argv, argv_buf);
    return 0;
}
