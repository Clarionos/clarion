#pragma once

#include <clintrinsics/basic.hpp>

#include <string_view>

namespace clintrinsics
{
    struct DBTag;
    struct TrxTag;
    struct CursorTag;
    struct BlobTag;

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("createCursor")]] void createCursor(TrxTag *trx, void *p, void (*f)(void *p, CursorTag *cursor));
        [[clang::import_module("clarion"), clang::import_name("cursorHasValue")]] bool cursorHasValue(CursorTag *cursor);
        [[clang::import_module("clarion"), clang::import_name("cursorValue")]] BlobTag *cursorValue(CursorTag *cursor);
        [[clang::import_module("clarion"), clang::import_name("cursorNext")]] void cursorNext(CursorTag *cursor, void *p, void (*f)(void *p));
        [[clang::import_module("clarion"), clang::import_name("closeCursor")]] void closeCursor(CursorTag *cursor);
    } // namespace imports

    struct KVRange : ExternalObject<CursorTag>
    {
        using ExternalObject<CursorTag>::ExternalObject;

        struct end_iterator
        {
        };

        struct iterator
        {
            KVRange &KVRange;

            bool operator!=(const end_iterator &) { return imports::cursorHasValue(KVRange.handle); }
            auto operator++()
            {
                return callExternalAsync<void, imports::cursorNext>(std::tuple{KVRange.handle}, [] {});
            }
            ExternalObject<BlobTag> operator*() { return imports::cursorValue(KVRange.handle); }
        };

        NoWait<iterator> begin() { return iterator{*this}; }
        end_iterator end() { return {}; }
    };

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("abortTransaction")]] void abortTransaction(TrxTag *trx);
        [[clang::import_module("clarion"), clang::import_name("commitTransaction")]] void commitTransaction(TrxTag *trx);
        [[clang::import_module("clarion"), clang::import_name("setKV")]] void setKV(TrxTag *trx, const void *key, uint32_t keyLen, const void *value, uint32_t valueLen, void *p, void (*f)(void *p));
    } // namespace imports

    struct transaction : ExternalObject<TrxTag>
    {
        bool committed = false;

        using ExternalObject<TrxTag>::ExternalObject;

        ~transaction()
        {
            if (!committed)
                abort();
        }

        void commit()
        {
            imports::commitTransaction(handle);
            committed = true;
        }

        void abort() { imports::abortTransaction(handle); }

        auto setKV(const void *key, uint32_t keyLen, const void *value, uint32_t valueLen)
        {
            return callExternalAsync<void, imports::setKV>(std::tuple{handle, key, keyLen, value, valueLen}, []() {});
        }

        auto setKV(std::string_view key, std::string_view value)
        {
            return setKV(key.begin(), key.size(), value.begin(), value.size());
        }

        auto everything()
        {
            return callExternalAsync<KVRange, imports::createCursor>(std::tuple{handle}, [](CursorTag *cursor) { return cursor; });
        }
    }; // transaction

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("openDb")]] void openDb(const char *name, uint32_t len, void *p, void (*f)(void *p, DBTag *db));
        [[clang::import_module("clarion"), clang::import_name("createTransaction")]] TrxTag *createTransaction(DBTag *db, bool writable);
        [[clang::import_module("clarion"), clang::import_name("closeDatabase")]] void closeDatabase(DBTag *db);
    } // namespace imports

    struct database : ExternalObject<DBTag>
    {
        using ExternalObject<DBTag>::ExternalObject;

        transaction createTransaction(bool writable)
        {
            return {imports::createTransaction(handle, writable)};
        }

        void close()
        {
            imports::closeDatabase(handle);
        }
    };

    auto openDb(std::string_view name)
    {
        return callExternalAsync<database, imports::openDb>(std::tuple{name.begin(), name.size()}, [](DBTag *db) { return db; });
    }

} // namespace clintrinsics
