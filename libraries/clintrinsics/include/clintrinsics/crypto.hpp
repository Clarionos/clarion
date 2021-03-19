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
      Sha256 toSha256() const
      {
         auto bytes = toUint8Vector();
         Sha256 sha256;
         if (bytes.size() != sizeof(sha256))
         {
            fatal("sha256: size mismatch");
         }
         memcpy(&sha256, bytes.data(), bytes.size());
         return sha256;
      }

      PublicKeyType toPublicKey(EccCurve curve) const
      {
         auto bytes = toUint8Vector();
         auto size = imports::getObjSize(handle);
         if (size != 33)
         {
            fatal("invalid public key");
         }

         PublicKeyType publicKey;
         switch (curve)
         {
            case EccCurve::k1:
               publicKey = EccPublicKey<k1>{};
               break;
            case EccCurve::r1:
               publicKey = EccPublicKey<r1>{};
               break;
            default:
               fatal("unknown key curve");
         }
         std::visit([&](auto&& keyValue) { imports::getObjData(handle, keyValue.data()); },
                    publicKey);
         return publicKey;
      }
   };

   namespace imports
   {
      [[clang::import_module("clarion"), clang::import_name("hash256")]] void
      hash256(const void* blob, uint32_t blobLen, void* p, void (*f)(void* p, BytesTag* sha256));

      [[clang::import_module("clarion"), clang::import_name("createKey")]] void
      createKey(EccCurve curve, void* p, void (*f)(void* p, BytesTag* bytesTag));

      // TODO:
      //    signature sign(public_key, hash);
      //    public_key recover(signature, hash, optional<public_key>);
      //    hash hash256(blob);
      //    shared_secret diffyhelman(public_key local, public_key remote);
      //    blob aesEncrypt(shared_secret, blob);
   }  // namespace imports

   auto hash256(const void* blob, uint32_t blobLen)
   {
      return callExternalAsync<Sha256, imports::hash256>(
          std::tuple{blob, blobLen},
          [](BytesTag* bytesTag) { return ExternalCryptoBytes{bytesTag}.toSha256(); });
   }

   auto createKey(EccCurve curve)
   {
      return callExternalAsync<PublicKeyType, imports::createKey>(
          std::tuple{curve},
          [curve](BytesTag* bytesTag) { return ExternalCryptoBytes{bytesTag}.toPublicKey(curve); });
   }

}  // namespace clintrinsics
