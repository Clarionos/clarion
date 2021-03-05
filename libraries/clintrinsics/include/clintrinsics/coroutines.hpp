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

    // TODO: name
    template <typename T>
    struct trivial_awaitable
    {
        T value;

        trivial_awaitable(T value) : value{std::move(value)} {}
        trivial_awaitable(const trivial_awaitable &) = delete;
        trivial_awaitable(trivial_awaitable &&) = default;

        trivial_awaitable &operator=(const trivial_awaitable &) = delete;
        trivial_awaitable &operator=(trivial_awaitable &&) = default;

        bool await_ready() { return true; }
        void await_suspend(coro::coroutine_handle<void>) {}
        T await_resume() { return std::move(value); }
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
}
