#include <clintrinsics/basic.hpp>

extern "C" __wasi_errno_t __wasi_fd_fdstat_get(__wasi_fd_t fd, __wasi_fdstat_t* stat)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_fdstat_get")))
{
   stat->fs_filetype = __WASI_FILETYPE_CHARACTER_DEVICE;
   stat->fs_flags = __WASI_FDFLAGS_APPEND;
   stat->fs_rights_base = __WASI_RIGHTS_FD_READ | __WASI_RIGHTS_FD_WRITE;
   stat->fs_rights_inheriting = 0;
   return 0;
}
