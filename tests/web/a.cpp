#include <cstdio>
#include <cstring>

extern "C"
{
    _Noreturn void clarion_exit(uint32_t code) __attribute__((__import_module__("clarion"),
                                                              __import_name__("exit")));

    void clarion_console(
        const void *buf, uint32_t size) __attribute__((__import_module__("clarion"),
                                                       __import_name__("console")));

    _Noreturn void clarion_unimplemented(const char *msg)
    {
        clarion_console(msg, strlen(msg));
        clarion_exit(1);
    }

    // Polyfill
    _Noreturn void __wasi_proc_exit(
        __wasi_exitcode_t code) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                               __import_name__("proc_exit")))
    {
        clarion_exit(code);
    }

    // Polyfill: all file handles are TTYs
    __wasi_errno_t __wasi_fd_fdstat_get(
        __wasi_fd_t fd,
        __wasi_fdstat_t *stat) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                              __import_name__("fd_fdstat_get")))
    {
        stat->fs_filetype = __WASI_FILETYPE_CHARACTER_DEVICE;
        stat->fs_flags = __WASI_FDFLAGS_APPEND;
        stat->fs_rights_base = __WASI_RIGHTS_FD_READ | __WASI_RIGHTS_FD_WRITE;
        stat->fs_rights_inheriting = 0;
        return 0;
    }

    // Polyfill: all file handles are TTYs
    __wasi_errno_t __wasi_fd_write(
        __wasi_fd_t fd,
        const __wasi_ciovec_t *iovs,
        size_t iovs_len,
        __wasi_size_t *nwritten) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                                __import_name__("fd_write")))
    {
        if (nwritten)
            *nwritten = 0;
        for (; iovs_len; --iovs_len, ++iovs)
        {
            clarion_console(iovs->buf, iovs->buf_len);
            if (nwritten)
                *nwritten += iovs->buf_len;
        }
        return 0;
    }

    __wasi_errno_t __wasi_fd_seek(
        __wasi_fd_t fd,
        __wasi_filedelta_t offset,
        __wasi_whence_t whence,
        __wasi_filesize_t *newoffset) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                                     __import_name__("fd_seek")))
    {
        clarion_unimplemented("__wasi_fd_seek unimplemented");
    }

    __wasi_errno_t __wasi_fd_close(
        __wasi_fd_t fd) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                       __import_name__("fd_close")))
    {
        clarion_unimplemented("__wasi_fd_close unimplemented");
    }
} // extern "C"

int main()
{
    printf("hello\nthere\nworld");
}
