#pragma once

#include <clintrinsics/basic.hpp>

#include <string>
#include <string_view>

namespace clintrinsics
{
   struct ConnectionTag;

   namespace imports
   {
      [[clang::import_module("clarion"), clang::import_name("connect")]] void connect(
          const char* name,
          uint32_t len,
          void* pOnMessage,
          void (*fOnMessage)(void* pOnMessage, BytesTag* bytesIndex),
          void* pOnClose,
          void (*fOnClose)(void* pOnClose, uint32_t code),
          void* pOnError,
          void (*fOnError)(void* pOnError),
          void* p,
          void (*f)(void* p, ConnectionTag* conn));

      [[clang::import_module("clarion"), clang::import_name("createConnection")]] void
      createConnection(const char* name,
                       uint32_t len,
                       void* p,
                       void (*f)(void* p, ConnectionTag* conn));

      [[clang::import_module("clarion"), clang::import_name("setupConnection")]] void
      setupConnection(ConnectionTag* conn,
                      void* pOnMessage,
                      void (*fOnMessage)(void* pOnMessage, BytesTag* bytesIndex),
                      void* pOnClose,
                      void (*fOnClose)(void* pOnClose, uint32_t code),
                      void* pOnError,
                      void (*fOnError)(void* pOnError));

      [[clang::import_module("clarion"), clang::import_name("sendMessage")]] void sendMessage(
          ConnectionTag* conn,
          const void* messagePos,
          uint32_t messageLen,
          void* p,
          void (*f)(void* p));

      [[clang::import_module("clarion"), clang::import_name("close")]] void close(
          ConnectionTag* conn);
   }  // namespace imports

   struct Connection : ExternalObject<ConnectionTag>
   {
      using ExternalObject<ConnectionTag>::ExternalObject;

      Connection(const std::string& uri) : uri{uri} {}

      std::string uri;

      // onOpen just indicates that the connection was opened
      std::function<void()> onOpen = [] {};

      // onMessage receives the message bytes
      std::function<void(ExternalBytes)> onMessage = [](ExternalBytes) {};

      // onClose receives the reason code
      std::function<void(uint32_t)> onClose = [](uint32_t) {};

      // onError just indicates that an error happened with the socket
      std::function<void()> onError = [] {};

      void close() { imports::close(handle); }

      auto sendMessage(const void* messagePos, uint32_t messageLen)
      {
         return callExternalAsync<void, imports::sendMessage>(
             std::tuple{handle, messagePos, messageLen}, []() {});
      }

      auto sendMessage(std::string_view message)
      {
         return sendMessage(message.begin(), message.size());
      }

      auto create()
      {
         return callExternalAsync<void, imports::createConnection>(
             std::tuple{uri.c_str(), uri.size()},
             [this](ConnectionTag* conn) { this->handle = conn; });
      }

      void setup()
      {
         imports::setupConnection(
             handle,
             this,  // onMessage event
             [](void* p, BytesTag* bytesIndex) {
                auto self = (Connection*)p;
                self->onMessage(ExternalBytes{bytesIndex});
             },
             this,  // onClose event
             [](void* p, uint32_t code) {
                auto self = (Connection*)p;
                self->onClose(code);
             },
             this,  // onError event
             [](void* p) {
                auto self = (Connection*)p;
                self->onError();
             });
      }

      auto connect()
      {
         return callExternalAsync<void, imports::connect>(
             std::tuple{uri.c_str(), uri.size(),
                        // onMessage event
                        this,
                        [](void* p, BytesTag* bytesIndex) {
                           auto self = (Connection*)p;
                           self->onMessage(ExternalBytes{bytesIndex});
                        },
                        // onClose event
                        this,
                        [](void* p, uint32_t code) {
                           auto self = (Connection*)p;
                           self->onClose(code);
                        },
                        // onError event
                        this,
                        [](void* p) {
                           auto self = (Connection*)p;
                           self->onError();
                        }},
             [this](ConnectionTag* conn) {
                this->handle = conn;
                this->onOpen();
             });
      }
   };

   struct ConnectionAcceptorTag;

   namespace imports
   {
      [[clang::import_module("clarion"),
        clang::import_name("createAcceptor")]] ConnectionAcceptorTag*
      createAcceptor(uint32_t port, const char* name, uint32_t len);

      [[clang::import_module("clarion"), clang::import_name("listenAcceptor")]] void listen(
          ConnectionAcceptorTag* acceptor,
          void* pOnConnection,
          void (*fOnConnection)(void* pOnConnection, ConnectionTag* connection));
   }  // namespace imports

   struct ConnectionAcceptor : ExternalObject<ConnectionAcceptorTag>
   {
      uint32_t port;
      std::string protocol;

      ConnectionAcceptor(uint32_t port, std::string protocol) : port{port}, protocol{protocol}
      {
         handle = imports::createAcceptor(port, protocol.c_str(), protocol.size());
      }

      std::function<void(std::shared_ptr<Connection>)> onConnection =
          [](std::shared_ptr<Connection>) {};

      void listen()
      {
         // call acceptor function to start listening
         imports::listen(handle, this, [](void* p, ConnectionTag* connection) {
            auto self = (ConnectionAcceptor*)p;
            self->onConnection(std::make_shared<Connection>(connection));
         });
      }
   };

}  // namespace clintrinsics
