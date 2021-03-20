#pragma once

#include <clintrinsics/basic.hpp>

#include <array>
#include <variant>

namespace clintrinsics
{
   using std::variant;

   struct Sha256 : public std::array<uint64_t, 4>
   {
      using array<uint64_t, 4>::array;

      char* data() const { return (char*)this; }
      size_t data_size() const { return 256 / 8; }

      std::string toString() const
      {
         auto d = (char*)this;
         auto s = sizeof(*this);
         std::string r;
         const char* to_hex = "0123456789abcdef";
         uint8_t* c = (uint8_t*)d;
         for (uint32_t i = 0; i < s; ++i)
            (r += to_hex[(c[i] >> 4)]) += to_hex[(c[i] & 0x0f)];
         return r;
      }
   };

   enum EccCurve
   {
      k1 = 0,
      r1 = 1
   };

   template <EccCurve curve = k1>
   struct EccPublicKey : public std::array<char, 33>
   {
      using std::array<char, 33>::array;
   };

   template <EccCurve curve = k1>
   struct EccSignature : public std::array<char, 65>
   {
      using array<char, 65>::array;
   };

   using PublicKeyType = variant<EccPublicKey<k1>, EccPublicKey<r1>>;
   using SignatureType = variant<EccSignature<k1>, EccSignature<r1>>;

   struct ExternalCryptoBytes : public ExternalBytes
   {
      Sha256 toSha256() const { return toArrayObject<Sha256>(); }

      template <EccCurve Curve>
      EccPublicKey<Curve> toPublicKey() const
      {
         return toArrayObject<EccPublicKey<Curve>>();
      }

      template <EccCurve Curve>
      EccSignature<Curve> toSignature() const
      {
         return toArrayObject<EccSignature<Curve>>();
      }
   };

   namespace imports
   {
      // todo: expose synchronous too
      [[clang::import_module("clarion"), clang::import_name("sha256")]] void
      sha256(const void* blob, uint32_t blobLen, void* p, void (*f)(void* p, BytesTag* sha256));

      [[clang::import_module("clarion"), clang::import_name("sha256Sync")]] BytesTag* sha256Sync(
          const void* blob,
          uint32_t blobLen);

      [[clang::import_module("clarion"), clang::import_name("createKey")]] void
      createKey(EccCurve curve, void* p, void (*f)(void* p, BytesTag* bytesTag));

      [[clang::import_module("clarion"), clang::import_name("sign")]] void sign(
          const void* publicKey,
          uint32_t publicKeyLen,
          const void* hash,
          uint32_t hashLen,
          void* p,
          void (*f)(void* p, BytesTag* bytesTag));

      // TODO:
      //  public_key recover(signature, hash, optional<public_key>);
      //  (sync if possible) shared_secret diffyhelman(public_key local, public_key remote);
      //  (sync if possible) blob aesEncrypt(shared_secret, blob);
   }  // namespace imports

   auto sha256(const void* blob, uint32_t blobLen)
   {
      return callExternalAsync<Sha256, imports::sha256>(
          std::tuple{blob, blobLen},
          [](BytesTag* bytesTag) { return ExternalCryptoBytes{bytesTag}.toSha256(); });
   }

   Sha256 sha256Sync(const void* blob, uint32_t blobLen)
   {
      auto bytesTag = imports::sha256Sync(blob, blobLen);
      return ExternalCryptoBytes{bytesTag}.toSha256();
   }

   template <EccCurve Curve>
   auto createKey()
   {
      return callExternalAsync<EccPublicKey<Curve>, imports::createKey>(
          std::tuple{Curve},
          [](BytesTag* bytesTag) { return ExternalCryptoBytes{bytesTag}.toPublicKey<Curve>(); });
   }

   template <EccCurve Curve>
   auto sign(const EccPublicKey<Curve>& publicKey, const Sha256& hash)
   {
      return callExternalAsync<EccSignature<Curve>, imports::sign>(
          std::tuple{publicKey.begin(), publicKey.size(), hash.data(), hash.data_size()},
          [](BytesTag* bytesTag) { return ExternalCryptoBytes{bytesTag}.toSignature<Curve>(); });
   }

}  // namespace clintrinsics
