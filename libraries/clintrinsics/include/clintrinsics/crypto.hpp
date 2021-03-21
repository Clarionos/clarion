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

   struct AesCbcIv : public std::array<char, 16>
   {
      using std::array<char, 16>::array;
   };

   struct EcdhSharedSecret : public std::array<char, 32>
   {
      using std::array<char, 32>::array;
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
      createKey(EccCurve curve, void* p, void (*f)(void* p, BytesTag* publicKey));

      [[clang::import_module("clarion"), clang::import_name("sign")]] void sign(
          const void* publicKey,
          uint32_t publicKeyLen,
          const void* hash,
          uint32_t hashLen,
          void* p,
          void (*f)(void* p, BytesTag* signature));

      [[clang::import_module("clarion"), clang::import_name("recover")]] void recover(
          EccCurve curve,
          const void* signature,
          uint32_t signatureLen,
          const void* hash,
          uint32_t hashLen,
          void* p,
          void (*f)(void* p, BytesTag* publicKey));

      [[clang::import_module("clarion"), clang::import_name("diffieHellman")]] void diffieHellman(
          EccCurve curve,
          const void* localPublicKey,
          uint32_t localPublicKeyLen,
          const void* remotePublicKey,
          uint32_t remotePublicKeyLen,
          void* p,
          void (*f)(void* p, BytesTag* ecdhSharedSecret));

      [[clang::import_module("clarion"), clang::import_name("randomAesCbIv")]] BytesTag*
      randomAesCbcIv();

      [[clang::import_module("clarion"), clang::import_name("aesCbcEncrypt")]] void aesCbcEncrypt(
          const void* sharedSecret,
          uint32_t sharedSecretLen,
          const void* iv,
          uint32_t ivLen,
          const void* blob,
          uint32_t blobLen,
          void* p,
          void (*f)(void* p, BytesTag* encryptedBlob));

      [[clang::import_module("clarion"), clang::import_name("aesCbcDecrypt")]] void aesCbcDecrypt(
          const void* sharedSecret,
          uint32_t sharedSecretLen,
          const void* iv,
          uint32_t ivLen,
          const void* encryptedBlob,
          uint32_t encryptedBlobLen,
          void* p,
          void (*f)(void* p, BytesTag* decryptedBlob));

      // TODO:
      //  public_key recover(signature, hash, optional<public_key>); // todo: optional publickey?
   }  // namespace imports

   auto sha256(const void* blob, uint32_t blobLen)
   {
      return callExternalAsync<Sha256, imports::sha256>(
          std::tuple{blob, blobLen},
          [](BytesTag* sha256) { return ExternalCryptoBytes{sha256}.toArrayObject<Sha256>(); });
   }

   Sha256 sha256Sync(const void* blob, uint32_t blobLen)
   {
      auto sha256 = imports::sha256Sync(blob, blobLen);
      return ExternalCryptoBytes{sha256}.toArrayObject<Sha256>();
   }

   template <EccCurve Curve>
   auto createKey()
   {
      return callExternalAsync<EccPublicKey<Curve>, imports::createKey>(
          std::tuple{Curve}, [](BytesTag* publicKey) {
             return ExternalCryptoBytes{publicKey}.toArrayObject<EccPublicKey<Curve>>();
          });
   }

   template <EccCurve Curve>
   auto sign(const EccPublicKey<Curve>& publicKey, const Sha256& hash)
   {
      return callExternalAsync<EccSignature<Curve>, imports::sign>(
          std::tuple{publicKey.begin(), publicKey.size(), hash.data(), hash.data_size()},
          [](BytesTag* signature) {
             return ExternalCryptoBytes{signature}.toArrayObject<EccSignature<Curve>>();
          });
   }

   template <EccCurve Curve>
   auto recover(const EccSignature<Curve>& signature, const Sha256& hash)
   {
      return callExternalAsync<EccPublicKey<Curve>, imports::recover>(
          std::tuple{Curve, signature.begin(), signature.size(), hash.data(), hash.data_size()},
          [](BytesTag* publicKey) {
             return ExternalCryptoBytes{publicKey}.toArrayObject<EccPublicKey<Curve>>();
          });
   }

   template <EccCurve Curve>
   auto diffieHellman(const EccPublicKey<Curve>& localPublicKey,
                      const EccPublicKey<Curve>& remotePublicKey)
   {
      return callExternalAsync<EcdhSharedSecret, imports::diffieHellman>(
          std::tuple{Curve, localPublicKey.begin(), localPublicKey.size(), remotePublicKey.begin(),
                     remotePublicKey.size()},
          [](BytesTag* ecdhSharedSecret) {
             return ExternalCryptoBytes{ecdhSharedSecret}.toArrayObject<EcdhSharedSecret>();
          });
   }

   AesCbcIv randomAesCbcIv()
   {
      auto iv = imports::randomAesCbcIv();
      return ExternalCryptoBytes{iv}.toArrayObject<AesCbcIv>();
   }

   auto aesCbcEncrypt(const EcdhSharedSecret& sharedSecret,
                      const AesCbcIv& iv,
                      const void* blob,
                      uint32_t blobLen)
   {
      return callExternalAsync<ExternalBytes, imports::aesCbcEncrypt>(
          std::tuple{sharedSecret.begin(), sharedSecret.size(), iv.begin(), iv.size(), blob,
                     blobLen},
          [](BytesTag* externalBytes) { return ExternalBytes{externalBytes}; });
   }

   auto aesCbcDecrypt(const EcdhSharedSecret& sharedSecret,
                      const AesCbcIv& iv,
                      const void* encryptedBlob,
                      uint32_t encryptedBlobLen)
   {
      return callExternalAsync<ExternalBytes, imports::aesCbcDecrypt>(
          std::tuple{sharedSecret.begin(), sharedSecret.size(), iv.begin(), iv.size(),
                     encryptedBlob, encryptedBlobLen},
          [](BytesTag* externalBytes) { return ExternalBytes{externalBytes}; });
   }

}  // namespace clintrinsics
