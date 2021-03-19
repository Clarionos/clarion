#pragma once

#include <clintrinsics/basic.hpp>

#include <string>
#include <string_view>
#include <unordered_set>

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

      Connection(const std::string& protocol,
                 const std::string& remoteAddress,
                 const uint32_t remotePort,
                 const std::string& localAddress = "",
                 const uint32_t localPort = 0)
          : protocol{protocol},
            remoteAddress{remoteAddress},
            remotePort{remotePort},
            localAddress{localAddress},
            localPort{localPort}
      {
      }

      std::string protocol;
      std::string remoteAddress;
      uint32_t remotePort;
      std::string localAddress;
      uint32_t localPort;

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

      // used to setup callbacks on connection events
      // (mostly for setup and handling new incoming connections)
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

      // used for creating and establishing a new client connection
      auto connect()
      {
         std::string uri = protocol + "://" + remoteAddress + ":" + std::to_string(remotePort);
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
          void (*fOnConnection)(void* pOnConnection,
                                ConnectionTag* connectionTag,
                                BytesTag* protocol,
                                BytesTag* remoteAddress,
                                uint32_t remotePort,
                                BytesTag* localAddress,
                                uint32_t localPort));
   }  // namespace imports

   struct ConnectionAcceptor : ExternalObject<ConnectionAcceptorTag>
   {
      uint32_t port;
      std::string protocol;

      std::function<void(std::shared_ptr<Connection>)> onConnection =
          [](std::shared_ptr<Connection>) {};

      ConnectionAcceptor(uint32_t port, std::string protocol) : port{port}, protocol{protocol}
      {
         handle = imports::createAcceptor(port, protocol.c_str(), protocol.size());
      }

      void listen()
      {
         // call acceptor function to start listening
         imports::listen(
             handle, this,
             [](void* p, ConnectionTag* connectionTag, BytesTag* protocol, BytesTag* remoteAddress,
                uint32_t remotePort, BytesTag* localAddress, uint32_t localPort) {
                auto self = (ConnectionAcceptor*)p;
                auto connection = std::make_shared<Connection>(connectionTag);
                connection->protocol = ExternalBytes{protocol}.toString();
                connection->remoteAddress = ExternalBytes{remoteAddress}.toString();
                connection->remotePort = remotePort;
                connection->localAddress = ExternalBytes{localAddress}.toString();
                connection->localPort = localPort;
                self->onConnection(connection);
             });
      }
   };

}  // namespace clintrinsics
