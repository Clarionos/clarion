#include <cstdio>
#include <cstring>
#include <experimental/coroutine>

namespace clarion
{
    namespace coro = std::experimental::coroutines_v1;

    [[clang::import_module("clarion"), clang::import_name("exit"), noreturn]] void exit(uint32_t code);

    [[clang::import_module("clarion"), clang::import_name("console")]] void console(
        const void *buf, uint32_t size);

    [[noreturn]] inline void fatal(const char *msg)
    {
        console(msg, strlen(msg));
        exit(1);
    }

    [[clang::import_module("clarion"), clang::import_name("callme_later")]] void callme_later(uint32_t delay_ms, void *p, void (*f)(void *));

    struct later
    {
        uint32_t delay_ms = 1000;

        bool await_ready() { return false; }

        void await_suspend(coro::coroutine_handle<void> co)
        {
            clarion::callme_later(delay_ms, co.address(), [](void *p) {
                coro::coroutine_handle<void>::from_address(p).resume();
            });
        }

        void await_resume() {}
    };

    struct task
    {
        struct promise_type
        {
            promise_type() { printf("promise_type::promise_type\n"); }
            ~promise_type() { printf("promise_type::~promise_type\n"); }
            auto initial_suspend() { return coro::suspend_always(); }
            auto final_suspend() noexcept { return coro::suspend_never(); }
            task get_return_object() { return {this}; }
            void unhandled_exception() { std::abort(); }
            void return_void() {}
        };

        promise_type *promise = nullptr;

        task() = default;
        task(promise_type *promise) : promise{promise} {}
        task(const task &) = delete;
        task(task &&src)
        {
            *this = std::move(src);
        }

        ~task()
        {
            if (promise)
                coro::coroutine_handle<promise_type>::from_promise(*promise).destroy();
        }

        task &operator=(const task &) = delete;
        task &operator=(task &&src)
        {
            promise = src.promise;
            src.promise = nullptr;
            return *this;
        }

        void start()
        {
            coro::coroutine_handle<promise_type>::from_promise(*promise).resume();
            promise = nullptr;
        }
    }; // task
} // namespace clarion

clarion::task testco(const char *s, uint32_t delay_ms)
{
    for (int i = 0; i < 10; ++i)
    {
        printf("s = \"%s\", i = %d\n", s, i);
        co_await clarion::later{delay_ms};
    }
    printf("s = %s, finished\n", s);
}

extern "C"
{
    // Polyfill
    _Noreturn void __wasi_proc_exit(
        __wasi_exitcode_t code) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                               __import_name__("proc_exit")))
    {
        clarion::exit(code);
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
            clarion::console(iovs->buf, iovs->buf_len);
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
        clarion::fatal("__wasi_fd_seek fatal");
    }

    __wasi_errno_t __wasi_fd_close(
        __wasi_fd_t fd) __attribute__((__import_module__("wasi_snapshot_preview1"),
                                       __import_name__("fd_close")))
    {
        clarion::fatal("__wasi_fd_close fatal");
    }
} // extern "C"

int main()
{
    printf("starting coroutines...\n");
    testco("delay 1s", 1000).start();
    testco("delay 2s", 2000).start();
    testco("delay 3s", 3000).start();
    printf("main returned\n");
}
