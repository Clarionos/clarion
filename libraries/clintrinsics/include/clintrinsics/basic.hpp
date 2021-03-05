#pragma once

#include "clintrinsics/coroutines.hpp"

#include <cstring>

namespace clintrinsics
{
    [[clang::import_module("clarion"), clang::import_name("exit"), noreturn]] void exit(uint32_t code);

    [[clang::import_module("clarion"), clang::import_name("console")]] void console(
        const void *buf, uint32_t size);

    [[noreturn]] inline void fatal(const char *msg)
    {
        console(msg, strlen(msg));
        exit(1);
    }

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("callme_later")]] void callme_later(uint32_t delay_ms, void *p, void (*f)(void *));
    }

    inline auto later(uint32_t delay_ms)
    {
        return call_external_async<void, imports::callme_later>(std::tuple{delay_ms}, [] {});
    }

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("release_object")]] void release_object(void *handle);
    }

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
                imports::release_object(handle);
        }

        external_object &operator=(const external_object &) = delete;
        external_object &operator=(external_object &&src)
        {
            handle = src.handle;
            src.handle = nullptr;
            return *this;
        }
    };
}
