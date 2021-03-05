#pragma once

#include <clintrinsics/basic.hpp>

#include <string_view>

namespace clintrinsics
{
    struct db_tag;
    struct trx_tag;
    struct cursor_tag;
    struct blob_tag;

    namespace imports
    {
        [[clang::import_module("clarion"), clang::import_name("open_db")]] void open_db(const char *name, uint32_t len, void *p, void (*f)(void *p, db_tag *db));
        [[clang::import_module("clarion"), clang::import_name("create_transaction")]] trx_tag *create_transaction(db_tag *db, bool writable);
        [[clang::import_module("clarion"), clang::import_name("abort_transaction")]] void abort_transaction(trx_tag *trx);
        [[clang::import_module("clarion"), clang::import_name("commit_transaction")]] void commit_transaction(trx_tag *trx);
        [[clang::import_module("clarion"), clang::import_name("set_kv")]] void set_kv(trx_tag *trx, const void *key, uint32_t keyLen, const void *value, uint32_t valueLen, void *p, void (*f)(void *p));
        [[clang::import_module("clarion"), clang::import_name("create_cursor")]] void create_cursor(trx_tag *trx, void *p, void (*f)(void *p, cursor_tag *cursor));
        [[clang::import_module("clarion"), clang::import_name("cursor_has_value")]] bool cursor_has_value(cursor_tag *cursor);
        [[clang::import_module("clarion"), clang::import_name("cursor_value")]] blob_tag *cursor_value(cursor_tag *cursor);
        [[clang::import_module("clarion"), clang::import_name("cursor_next")]] void cursor_next(cursor_tag *cursor, void *p, void (*f)(void *p));
    } // namespace imports

    struct kv_range : external_object<cursor_tag>
    {
        using external_object<cursor_tag>::external_object;

        struct end_iterator
        {
        };

        struct iterator
        {
            kv_range &kv_range;

            bool operator!=(const end_iterator &) { return imports::cursor_has_value(kv_range.handle); }
            auto operator++()
            {
                return call_external_async<void, imports::cursor_next>(std::tuple{kv_range.handle}, [] {});
            }
            external_object<blob_tag> operator*() { return imports::cursor_value(kv_range.handle); }
        };

        trivial_awaitable<iterator> begin() { return iterator{*this}; }
        end_iterator end() { return {}; }
    };

    struct transaction : external_object<trx_tag>
    {
        bool committed = false;

        using external_object<trx_tag>::external_object;

        ~transaction()
        {
            if (!committed)
                abort();
        }

        void commit()
        {
            imports::commit_transaction(handle);
            committed = true;
        }

        void abort() { imports::abort_transaction(handle); }

        auto set_kv(const void *key, uint32_t keyLen, const void *value, uint32_t valueLen)
        {
            return call_external_async<void, imports::set_kv>(std::tuple{handle, key, keyLen, value, valueLen}, []() {});
        }

        auto set_kv(std::string_view key, std::string_view value)
        {
            return set_kv(key.begin(), key.size(), value.begin(), value.size());
        }

        auto everything()
        {
            return call_external_async<kv_range, imports::create_cursor>(std::tuple{handle}, [](cursor_tag *cursor) { return cursor; });
        }
    }; // transaction

    struct database : external_object<db_tag>
    {
        using external_object<db_tag>::external_object;

        transaction create_transaction(bool writable)
        {
            return {imports::create_transaction(handle, writable)};
        }
    };

    auto open_db(std::string_view name)
    {
        return call_external_async<database, imports::open_db>(std::tuple{name.begin(), name.size()}, [](db_tag *db) { return db; });
    }

} // namespace clintrinsics
