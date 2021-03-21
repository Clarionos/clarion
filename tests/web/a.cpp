#include "clintrinsics/connection.hpp"
#include "clintrinsics/crypto.hpp"
#include "clintrinsics/database.hpp"

#include <string>

clintrinsics::Task<std::string> testco(std::string s, uint32_t delayMillisec)
{
   std::string result;
   for (int i = 0; i < 10; ++i)
   {
      printf("s = \"%s\", i = %d\n", s.c_str(), i);
      result += "(" + s + ")";
      co_await clintrinsics::later(delayMillisec);
   }
   printf("s = \" %s \", finished\n", s.c_str());
   co_return result;
}

clintrinsics::Task<> testco2(uint32_t delayMillisec)
{
   printf("loop 1 returned: %s\n", (co_await testco("loop 1", delayMillisec)).c_str());
   printf("loop 2 returned: %s\n", (co_await testco("loop 2", delayMillisec)).c_str());
   printf("testco2 finished\n");
}

clintrinsics::Task<> testDb()
{
   auto db = co_await clintrinsics::openDb("foo");
   printf(">> database handle: %p\n", db.handle);
   auto trx = db.createTransaction(true);
   printf(">> trx handle: %p\n", trx.handle);
   co_await trx.setKV("abcd", "efgh");
   co_await trx.setKV("ijkl", "mnop");

    for
       co_await(auto x : co_await trx.everything()) printf("... blob handle %p\n", x.handle);

    trx.commit();
    db.close();
}

clintrinsics::Task<> testNet()
{
   clintrinsics::Connection myConnection{"ws", "localhost", 9125};
   myConnection.onOpen = []() { printf("connection opened!\n"); };
   myConnection.onMessage = [](clintrinsics::ExternalBytes data) {
      printf("received bytes handle: %p -- size: %d\n", data.handle,
             (int)data.toUint8Vector().size());
   };
   myConnection.onClose = [](uint32_t code) { printf("connection closed code: %d\n", code); };
   myConnection.onError = []() { printf("client connection has failed!\n"); };
   co_await myConnection.connect();

   printf(">> connection handle: %p\n", myConnection.handle);
   co_await myConnection.sendMessage("hey there from CLARION wasm! :)\n");  // todo: make send async
   printf(">> message sent!\n");

   co_await clintrinsics::later(5000);  // waits for some messages exchanges before closing it

   myConnection.close();
   printf(">> connection closed!\n");
}

clintrinsics::Task<> testCrypto()
{
   printf("testing hashing...\n");
   std::string message = "hey ho lets go!";
   auto sha256 = co_await clintrinsics::sha256(message.c_str(), message.size());
   printf(">> sha256 message size: %d - %s \n", (int)sha256.size(), sha256.toString().c_str());
   if (sha256.toString() != "d37382f283e176ca61031f07fbd857cb5a7dbfc4b327ef59654d5f9bf5022291")
   {
      clintrinsics::fatal("sha256 incorrect hash\n");
   }

   printf("testing synchronous hashing...\n");
   std::string messageSync = "hey ho lets go! sync";
   auto sha256Sync = clintrinsics::sha256Sync(messageSync.c_str(), messageSync.size());
   printf(">> sha256Sync message size: %d - %s \n", (int)sha256Sync.size(),
          sha256Sync.toString().c_str());
   if (sha256Sync.toString() != "ff0403bbd051115a255e5bc9fccc4b90c2906c76f0601bcd4a11ad242f91efac")
   {
      clintrinsics::fatal("sha256 incorrect hash\n");
   }

   printf("creating new K1 public key...\n");
   auto k1PubKey = co_await clintrinsics::createKey<clintrinsics::EccCurve::k1>();
   printf("k1 pubKey third byte >> %c\n", k1PubKey[2]);

   printf("creating new r1 public key...\n");
   auto r1PubKey = co_await clintrinsics::createKey<clintrinsics::EccCurve::r1>();
   printf("k1 pubKey third byte >> %c\n", r1PubKey[2]);

   auto signatureK1 = co_await clintrinsics::sign<clintrinsics::EccCurve::k1>(k1PubKey, sha256);
   printf("signedk1 message third byte >> %c\n", signatureK1[2]);

   auto recoveredK1 = co_await clintrinsics::recover(signatureK1, sha256);
   if (recoveredK1 != k1PubKey)
   {
      clintrinsics::fatal("fail to recover K1pubkey\n");
   }
   else
   {
      printf("recovered K1 key from signature K1!\n");
   }

   auto signatureR1 = co_await clintrinsics::sign<clintrinsics::EccCurve::r1>(r1PubKey, sha256);
   printf("signedr1 message third byte >> %c\n", signatureR1[2]);

   auto recoveredR1 = co_await clintrinsics::recover(signatureR1, sha256);
   if (recoveredR1 != r1PubKey)
   {
      clintrinsics::fatal("fail to recover R1pubkey\n");
   }
   else
   {
      printf("recovered R1 key from signature R1!\n");
   }

   auto remoteK1PubKey = co_await clintrinsics::createKey<clintrinsics::EccCurve::k1>();
   auto sharedSecret1 =
       co_await clintrinsics::diffieHellman<clintrinsics::EccCurve::k1>(k1PubKey, remoteK1PubKey);
   printf("got diffie hellmann shared secret!\n");

   std::string topSecretMessage = "Clarion is going to change the world!";
   auto iv1 = clintrinsics::randomAesCbcIv();
   auto encryptedBlob = co_await clintrinsics::aesCbcEncrypt(
       sharedSecret1, iv1, topSecretMessage.c_str(), topSecretMessage.size());

   // emulating a remote shared secret creation, it must be equal to the above generated secret
   auto sharedSecret1FromRemote =
       co_await clintrinsics::diffieHellman<clintrinsics::EccCurve::k1>(remoteK1PubKey, k1PubKey);
   if (sharedSecret1FromRemote != sharedSecret1)
   {
      clintrinsics::fatal("shared secret mismatch when generated from remote\n");
   }
   else
   {
      printf("got diffie hellman shared secret on remote\n");
   }

   // emulating the remote decryption
   auto encryptedBlobBytes = encryptedBlob.toUint8Vector();
   auto decryptedBlob = co_await clintrinsics::aesCbcDecrypt(
       sharedSecret1FromRemote, iv1, encryptedBlobBytes.data(), encryptedBlobBytes.size());
   auto decryptedMessage = decryptedBlob.toString();
   printf("decrypted message: %s\n", decryptedMessage.c_str());
   if (decryptedMessage != topSecretMessage)
   {
      clintrinsics::fatal("decrypt message mismatch\n");
   }
   else
   {
      printf("decrypted top secret message successfully!\n");
   }
}

// todo: move to a proper node file
namespace clintrinsics
{
   struct ClarionNode
   {
      std::unordered_set<std::shared_ptr<ConnectionAcceptor>> acceptors;
      std::unordered_set<std::shared_ptr<Connection>> connections;

      void addConnection(std::shared_ptr<Connection> connection)
      {
         connections.emplace(connection);
      }

      void removeConnection(std::shared_ptr<Connection> connection)
      {
         connections.erase(connection);
      }

      void status()
      {
         printf(">>> Acceptors %d / Connections: %d\n", (int)acceptors.size(),
                (int)connections.size());
      }
   };
}  // namespace clintrinsics

static clintrinsics::ClarionNode node;
static std::shared_ptr<clintrinsics::ConnectionAcceptor> globalAcceptor;

void setupGlobalAcceptor()
{
   globalAcceptor = std::make_shared<clintrinsics::ConnectionAcceptor>(9125, "ws");
   node.acceptors.emplace(globalAcceptor);

   globalAcceptor->onConnection = [](std::shared_ptr<clintrinsics::Connection> connection) {
      printf("conn(%d/%p) >>> new incoming connection: ([%s] remote: %s:%d / local: %s:%d)\n",
             globalAcceptor->port, connection->handle, connection->protocol.c_str(),
             connection->remoteAddress.c_str(), connection->remotePort,
             connection->localAddress.c_str(), connection->localPort);
      connection->onMessage = [connection](clintrinsics::ExternalBytes data) {
         auto dataBytes = data.toUint8Vector();
         printf("conn(%d/%p) >>> received bytes handle: %p -- size: %d\n", globalAcceptor->port,
                connection->handle, data.handle, (int)dataBytes.size());
         connection->sendMessage(dataBytes.data(), dataBytes.size()).run();
      };
      connection->onClose = [connection](uint32_t code) {
         printf("conn(%d/%p) >>> connection closed code: %d\n", globalAcceptor->port,
                connection->handle, code);
         node.removeConnection(connection);
      };
      connection->onError = [connection]() {
         printf("conn(%d/%p) >>> connection failed with error!\n", globalAcceptor->port,
                connection->handle);
      };
      connection->setup();
      connection->sendMessage("welcome to a world with Clarity").run();

      printf("conn(%d/%p) >>> setup done!\n", globalAcceptor->port, connection->handle);

      node.addConnection(connection);
   };
   globalAcceptor->listen();
   printf("global acceptor handle >> %p\n", globalAcceptor->handle);
}

[[clang::export_name("initServer")]] void initServer()
{
   printf("initializing clariond server...\n");
   setupGlobalAcceptor();
   printf("clariond initialization server returned\n");
}

[[clang::export_name("status")]] void status()
{
   node.status();
}

[[clang::export_name("test")]] void test()
{
   // testco("delay 1s", 1000).start();
   // testco("delay 2s", 2000).start();
   // testco("delay 3s", 3000).start();
   // testco2(200).start();
   // testco2(250).start();
   // testDb().start();
   // testNet().start();
   testCrypto().start();
}

int main()
{
   printf("starting coroutines...\n");
   test();
   printf("main returned\n");
}
