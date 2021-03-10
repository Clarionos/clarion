#pragma once

#include <cstdio>
#include <experimental/coroutine>
#include <functional>
#include <tuple>

// TODO: move non-intrinsic pieces to another lib

namespace clintrinsics
{
    namespace coro = std::experimental::coroutines_v1;

    template <typename Ret = void>
    struct Task;

    template <typename Ret, typename Derived>
    struct TaskPromiseBase
    {
        coro::coroutine_handle<void> continuation;

        TaskPromiseBase() { printf("TaskPromise::TaskPromise\n"); }
        ~TaskPromiseBase() { printf("TaskPromise::~TaskPromise\n"); }
        auto initial_suspend() { return coro::suspend_always(); }
        auto final_suspend() noexcept
        {
            if (continuation)
                continuation.resume();
            return coro::suspend_never();
        }
        Task<Ret> get_return_object() { return {(Derived *)this}; }
        void unhandled_exception() { std::abort(); }
    };

    template <typename Ret>
    struct TaskPromise : TaskPromiseBase<Ret, TaskPromise<Ret>>
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
    struct TaskPromise<void> : TaskPromiseBase<void, TaskPromise<void>>
    {
        void return_void() {}
    };

    template <typename Ret>
    struct TaskBase
    {
        using promise_type = TaskPromise<Ret>;

        promise_type *promise = nullptr;

        TaskBase() = default;
        TaskBase(promise_type *promise) : promise{promise} {}
        TaskBase(const TaskBase &) = delete;
        TaskBase(TaskBase &&src)
        {
            *this = std::move(src);
        }

        ~TaskBase()
        {
            if (promise)
                coro::coroutine_handle<promise_type>::from_promise(*promise).destroy();
        }

        TaskBase &operator=(const TaskBase &) = delete;
        TaskBase &operator=(TaskBase &&src)
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
    }; // TaskBase

    // A task is a coroutine which supports async operations. Once created, a task may be:
    //  * started exactly once using start(), or
    //  * co_awaited on by another task exactly once, or
    //  * be destroyed
    //
    // Starting a task or co_awaiting on a task detaches ownership from the task
    // object; the running task is automatically destroyed when it's done executing.
    template <typename Ret>
    struct Task : TaskBase<Ret>
    {
        using TaskBase<Ret>::TaskBase;

        Ret retval = {};

        void await_suspend(coro::coroutine_handle<void> co)
        {
            this->promise->retval = &retval;
            TaskBase<Ret>::await_suspend(co);
        }

        Ret await_resume() { return std::move(retval); }
    }; // Task

    template <>
    struct Task<void> : TaskBase<void>
    {
        using TaskBase<void>::TaskBase;

        void await_resume() {}
    };

    // An awaitable that already as a (single-use) value
    template <typename T>
    struct NoWait
    {
        T value;

        NoWait(T value) : value{std::move(value)} {}
        NoWait(const NoWait &) = delete;
        NoWait(NoWait &&) = default;

        NoWait &operator=(const NoWait &) = delete;
        NoWait &operator=(NoWait &&) = default;

        bool await_ready() { return true; }
        void await_suspend(coro::coroutine_handle<void>) {}
        T await_resume() { return std::move(value); }
    };

    // An awaitable which invokes an external async function, then feeds
    // the result through a lambda
    template <typename Ret, auto extFn, typename Args, typename Lambda>
    struct CallAsyncAwaitable
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
                auto self = (CallAsyncAwaitable *)p;
                self->result = std::invoke(self->lambda, result...);
                self->co.resume();
            };
            std::apply(extFn, std::tuple_cat(args, std::tuple{this, post_process}));
        }

        Ret await_resume() { return std::move(result); }
    };

    template <auto extFn, typename Args, typename Lambda>
    struct CallAsyncAwaitable<void, extFn, Args, Lambda>
    {
        Args args;
        Lambda lambda;
        coro::coroutine_handle<void> co;

        bool await_ready() { return false; }

        void await_suspend(coro::coroutine_handle<void> co)
        {
            this->co = co;
            auto post_process = [](void *p, auto... result) {
                auto self = (CallAsyncAwaitable *)p;
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
    auto callExternalAsync(Args args, Lambda lambda)
    {
        return CallAsyncAwaitable<Ret, extFn, Args, Lambda>{std::move(args), std::move(lambda)};
    }
}
