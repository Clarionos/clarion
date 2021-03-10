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
        [[clang::import_module("clarion"), clang::import_name("callmeLater")]] void callmeLater(uint32_t delayMillisec, void *p, void (*f)(void *));
    }

    inline auto later(uint32_t delayMillisec)
    {
        return callExternalAsync<void, imports::callmeLater>(std::tuple{delayMillisec}, [] {});
    }

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("releaseObject")]] void releaseObject(void *handle);
    }

    template <typename Tag>
    struct ExternalObject
    {
        Tag *handle = nullptr;

        ExternalObject() = default;
        ExternalObject(Tag *handle) : handle{handle} {}
        ExternalObject(const ExternalObject &) = delete;
        ExternalObject(ExternalObject &&src) { *this = std::move(src); }

        ~ExternalObject()
        {
            if (handle)
                imports::releaseObject(handle);
        }

        ExternalObject &operator=(const ExternalObject &) = delete;
        ExternalObject &operator=(ExternalObject &&src)
        {
            handle = src.handle;
            src.handle = nullptr;
            return *this;
        }
    };
}
