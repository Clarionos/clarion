#include <cstdio>
#include <cstring>
#include <experimental/coroutine>
#include <functional>
#include <string_view>
#include <string>
#include <tuple>

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

    template <typename Ret = void>
    struct task;

    template <typename Ret, typename Derived>
    struct task_promise_base
    {
        coro::coroutine_handle<void> continuation;

        task_promise_base() { printf("task_promise::task_promise\n"); }
        ~task_promise_base() { printf("task_promise::~task_promise\n"); }
        auto initial_suspend() { return coro::suspend_always(); }
        auto final_suspend() noexcept
        {
            if (continuation)
                continuation.resume();
            return coro::suspend_never();
        }
        task<Ret> get_return_object() { return {(Derived *)this}; }
        void unhandled_exception() { std::abort(); }
    };

    template <typename Ret>
    struct task_promise : task_promise_base<Ret, task_promise<Ret>>
    {
        Ret *retval = nullptr;

        template <typename T>
        void return_value(T &&value)
        {
            if (retval)
                *retval = std::forward<T>(value);
        }
    };

    template <>
    struct task_promise<void> : task_promise_base<void, task_promise<void>>
    {
        void return_void() {}
    };

    template <typename Ret>
    struct task_base
    {
        using promise_type = task_promise<Ret>;

        promise_type *promise = nullptr;

        task_base() = default;
        task_base(promise_type *promise) : promise{promise} {}
        task_base(const task_base &) = delete;
        task_base(task_base &&src)
        {
            *this = std::move(src);
        }

        ~task_base()
        {
            if (promise)
                coro::coroutine_handle<promise_type>::from_promise(*promise).destroy();
        }

        task_base &operator=(const task_base &) = delete;
        task_base &operator=(task_base &&src)
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

        bool await_ready() { return false; }

        // TODO: should return a coro::coroutine_handle<void> to enable efficient task switching,
        // but that causes clang 11.0.0 to report an error: "WebAssembly 'tail-call' feature not enabled".
        // This will also require changes to promise_type::final_suspend().
        void await_suspend(coro::coroutine_handle<void> co)
        {
            promise->continuation = co;
            coro::coroutine_handle<promise_type>::from_promise(*promise).resume();
            promise = nullptr;
        }
    }; // task_base

    // A task is a coroutine which supports async operations. Once created, a task may be:
    //  * started exactly once using start(), or
    //  * co_awaited on by another task exactly once, or
    //  * be destroyed
    //
    // Starting a task or co_awaiting on a task detaches ownership from the task
    // object; the running task is automatically destroyed when it's done executing.
    template <typename Ret>
    struct task : task_base<Ret>
    {
        using task_base<Ret>::task_base;

        Ret retval = {};

        void await_suspend(coro::coroutine_handle<void> co)
        {
            this->promise->retval = &retval;
            task_base<Ret>::await_suspend(co);
        }

        Ret await_resume() { return std::move(retval); }
    }; // task

    template <>
    struct task<void> : task_base<void>
    {
        using task_base<void>::task_base;

        void await_resume() {}
    };

    // An awaitable which invokes an external async function, then feeds
    // the result through a lambda
    template <typename Ret, auto extFn, typename Args, typename Lambda>
    struct call_async_awaitable
    {
        Args args;
        Lambda lambda;
        coro::coroutine_handle<void> co;
        Ret result;

        bool await_ready() { return false; }

        void await_suspend(coro::coroutine_handle<void> co)
        {
            this->co = co;
            auto post_process = [](void *p, auto... result) {
                auto self = (call_async_awaitable *)p;
                self->result = std::invoke(self->lambda, result...);
                self->co.resume();
            };
            std::apply(extFn, std::tuple_cat(args, std::tuple{this, post_process}));
        }

        Ret await_resume() { return std::move(result); }
    };

    template <auto extFn, typename Args, typename Lambda>
    struct call_async_awaitable<void, extFn, Args, Lambda>
    {
        Args args;
        Lambda lambda;
        coro::coroutine_handle<void> co;

        bool await_ready() { return false; }

        void await_suspend(coro::coroutine_handle<void> co)
        {
            this->co = co;
            auto post_process = [](void *p, auto... result) {
                auto self = (call_async_awaitable *)p;
                std::invoke(self->lambda, result...);
                self->co.resume();
            };
            std::apply(extFn, std::tuple_cat(args, std::tuple{this, post_process}));
        }

        void await_resume() {}
    };

    // Return an awaitable which invokes an external async function, then feeds
    // the result through a lambda
    template <typename Ret, auto extFn, typename Args, typename Lambda>
    auto call_external_async(Args args, Lambda lambda)
    {
        return call_async_awaitable<Ret, extFn, Args, Lambda>{std::move(args), std::move(lambda)};
    }

    [[clang::import_module("clarion"), clang::import_name("callme_later")]] void callme_later(uint32_t delay_ms, void *p, void (*f)(void *));
    inline auto later(uint32_t delay_ms)
    {
        return call_external_async<void, callme_later>(std::tuple{delay_ms}, [] {});
    }

    [[clang::import_module("clarion"), clang::import_name("release_object")]] void release_object(void *handle);

    template <typename Tag>
    struct external_object
    {
        Tag *handle = nullptr;

        external_object() = default;
        external_object(Tag *handle) : handle{handle} {}
        external_object(const external_object &) = delete;
        external_object(external_object &&src) { *this = std::move(src); }

        ~external_object()
        {
            if (handle)
                release_object(handle);
        }

        external_object &operator=(const external_object &) = delete;
        external_object &operator=(external_object &&src)
        {
            handle = src.handle;
            src.handle = nullptr;
            return *this;
        }
    };

    struct db_tag;
    [[clang::import_module("clarion"), clang::import_name("open_db")]] void open_db_raw(const char *name, uint32_t len, void *p, void (*f)(void *p, db_tag *db));

    struct database : external_object<db_tag>
    {
        using external_object<db_tag>::external_object;
    };

    auto open_db(std::string_view name)
    {
        return call_external_async<database, open_db_raw>(std::tuple{name.begin(), name.size()}, [](db_tag *db) { return db; });
    }

} // namespace clarion

clarion::task<std::string> testco(std::string s, uint32_t delay_ms)
{
    std::string result;
    for (int i = 0; i < 10; ++i)
    {
        printf("s = \"%s\", i = %d\n", s.c_str(), i);
        result += "(" + s + ")";
        co_await clarion::later(delay_ms);
    }
    printf("s = \" %s \", finished\n", s.c_str());
    co_return result;
}

clarion::task<> testco2(uint32_t delay_ms)
{
    printf("loop 1 returned: %s\n", (co_await testco("loop 1", delay_ms)).c_str());
    printf("loop 2 returned: %s\n", (co_await testco("loop 2", delay_ms)).c_str());
    printf("testco2 finished\n");
}

clarion::task<> testdb()
{
    auto db = co_await clarion::open_db("foo");
    printf("database handle: %p\n", db.handle);
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
    //testco("delay 1s", 1000).start();
    //testco("delay 2s", 2000).start();
    //testco("delay 3s", 3000).start();
    // testco2(200).start();
    // testco2(250).start();
    testdb().start();
    printf("main returned\n");
}
