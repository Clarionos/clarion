#pragma once

#include <clintrinsics/basic.hpp>

#include <string_view>

namespace clintrinsics
{
    struct ConnectionTag;

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("connect")]] void connect(const char *name, uint32_t len, void *p, void (*f)(void *p, ConnectionTag *conn));

        [[clang::import_module("clarion"), clang::import_name("sendMessage")]] void sendMessage(ConnectionTag *conn, const void *messagePos, uint32_t messageLen);

        [[clang::import_module("clarion"), clang::import_name("close")]] void close(ConnectionTag *conn);
    } // namespace imports

    struct connection : ExternalObject<ConnectionTag>
    {
        using ExternalObject<ConnectionTag>::ExternalObject;

        void close()
        {
            imports::close(handle);
        }

        void sendMessage(const void *messagePos, uint32_t messageLen)
        {
            imports::sendMessage(handle, messagePos, messageLen);
        }

        auto sendMessage(std::string_view message)
        {
            return sendMessage(message.begin(), message.size());
        }
    };

    auto connect(std::string_view uri)
    {
        return callExternalAsync<connection, imports::connect>(std::tuple{uri.begin(), uri.size()}, [](ConnectionTag *conn) { return conn; });
    }

} // namespace clintrinsics
