#pragma once

#include "clintrinsics/coroutines.hpp"

#include <array>
#include <cstring>
#include <string>
#include <vector>

namespace clintrinsics
{
   [[clang::import_module("clarion"), clang::import_name("exit"), noreturn]] void exit(
       uint32_t code);

   [[clang::import_module("clarion"), clang::import_name("console")]] void console(const void* buf,
                                                                                   uint32_t size);

   [[noreturn]] inline void fatal(const char* msg)
   {
      console(msg, strlen(msg));
      exit(1);
   }

   namespace imports
   {
      [[clang::import_module("clarion"), clang::import_name("callmeLater")]] void
      callmeLater(uint32_t delayMillisec, void* p, void (*f)(void*));
   }

   inline auto later(uint32_t delayMillisec)
   {
      return callExternalAsync<void, imports::callmeLater>(std::tuple{delayMillisec}, [] {});
   }

   namespace imports
   {
      [[clang::import_module("clarion"), clang::import_name("releaseObject")]] void releaseObject(
          void* handle);
   }

   template <typename Tag>
   struct ExternalObject
   {
      Tag* handle = nullptr;

      ExternalObject() = default;
      ExternalObject(Tag* handle) : handle{handle} {}
      ExternalObject(const ExternalObject&) = delete;
      ExternalObject(ExternalObject&& src) { *this = std::move(src); }

      ~ExternalObject()
      {
         if (handle)
            imports::releaseObject(handle);
      }

      ExternalObject& operator=(const ExternalObject&) = delete;
      ExternalObject& operator=(ExternalObject&& src)
      {
         handle = src.handle;
         src.handle = nullptr;
         return *this;
      }
   };

   struct BytesTag;

   namespace imports
   {
      [[clang::import_module("clarion"), clang::import_name("getObjSize")]] uint32_t getObjSize(
          BytesTag* index);

      [[clang::import_module("clarion"), clang::import_name("getObjData")]] void getObjData(
          BytesTag* index,
          void* dest);
   }  // namespace imports

   struct ExternalBytes : ExternalObject<BytesTag>
   {
      std::vector<uint8_t> toUint8Vector() const
      {
         auto size = imports::getObjSize(handle);
         std::vector<uint8_t> data(size);
         imports::getObjData(handle, data.data());
         return data;
      }

      std::string toString() const
      {
         auto size = imports::getObjSize(handle);
         std::string str;
         str.reserve(size);
         imports::getObjData(handle, str.data());
         return str;
      }

      template <typename T>
      T toArrayObject() const
      {
         auto size = imports::getObjSize(handle);

         T array;
         if (size != sizeof(array))
         {
            fatal("to array memory size mismatch\n");
         }

         imports::getObjData(handle, array.data());
         return array;
      }
   };
}  // namespace clintrinsics
