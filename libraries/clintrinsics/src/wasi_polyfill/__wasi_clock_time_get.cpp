#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_clock_time_get(
    __wasi_clockid_t id,
    __wasi_timestamp_t precision,
    __wasi_timestamp_t *time) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                             __import_name__("clock_time_get")))
{
    *time = 0; // TODO
    return 0;
}
