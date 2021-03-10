#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_fd_seek(
    __wasi_fd_t fd,
    __wasi_filedelta_t offset,
    __wasi_whence_t whence,
    __wasi_filesize_t *newoffset) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                                 __import_name__("fd_seek")))
{
    clintrinsics::fatal("__wasi_fd_seek not implemented");
}
