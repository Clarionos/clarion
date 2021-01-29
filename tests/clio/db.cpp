#include <boost/core/demangle.hpp>
#include <iostream>

#include <clio/to_json/varint.hpp>
#include <clio/from_json/varint.hpp>
#include <clio/from_bin/varint.hpp>
#include <clio/to_bin/varint.hpp>
#include <clio/json/any.hpp>
#include <clio/compress.hpp>

#include <clio/translator.hpp>
#include <clio/to_json/map.hpp>
#include <clio/bytes/to_json.hpp>
#include <clio/bytes/from_json.hpp>
#include <fstream>

#include <clio/flatbuf.hpp>
#include <chrono>


class intrinsic {
    public:
        struct db_handle {
            uint32_t id;
        };
        enum error_code {
            ok                   = 0, /// everything is ok
            invalid_handle       = 1, /// the handle is unknwon to us
            freed_handle         = 2, /// the handle requested has already been freed
            invalid_buffer_pram  = 3,
            invalid_size_param   = 4,
            invalid_handle_param = 5  /// the user provided an invalid handle paramter ref
        };
        
        error_code db_create( size_t size, db_handle* new_handle ) {
            if( size <= 0 ) return invalid_size_param;
            if( new_handle == nullptr ) return invalid_handle_param;

            _database.emplace_back( buffer, buffer+size );

            auto& rec = _database.back();
            rec.resize(size);

            new_handle->id = _database.size() -1;
            return ok;
        }

        error_code db_set( db_handle h, const char* buffer, size_t size ) {
            if( size <= 0 ) return invalid_size_param;
            if( h.id < _database.size() ) {
                auto& rec = _database[h.id];
                if( rec.size() == 0 ) return freed_handle;
                rec.resize(size);
                memcpy( rec.data(), buffer, size );
                return ok;
            } else return invalid_handle;
        }

        size_t     db_get_size( db_handle h ) {
            if( h.id < _database.size() ) {
                return _database[h.id].size();
            } else return invalid_handle;
        }

        error_code db_get( db_handle h, char* buffer, size_t size_in) {
            if( h.id < _database.size() ) {
                auto& rec = _database[h.id];
                if( buffer && size_in ) {
                    auto read_size = std::min( size_in, rec.size() );
                    memcpy( buffer, rec.data(), read_size );
                } else { return invalid_buffer_pram; }
            } else return invalid_handle;
        }

        error_code db_free( db_handle h ) {
            if( h.id >= _database.size() ) return invalid_handle_param;
            _free_list.push_back( std::make_pair( h, std::move(_database[h.id]) ) );
            return ok;
        }

    private:
        std::vector< std::pair<uint32_t id, std::vector<char> > _free_list;
        std::vector< std::vector<char> >                        _database;
};

class cache {
    public:
        class cache_record {
            public:
                size_t      size()const     { return buffer.size(); }
                const char* data()const     { return buffer.data(); } 
                bool        is_dirty()const { return _dirty;        }
                db_handle   handle()const   { return _handle;       }

                template< typename M>
                cache_record& modify( M&& mod ) {
                    _dirty = true;
                    mod( _buffer );
                    return *this;
                }

                cache_record( cache_record&& mv )
                    :_cache(mv._cache),
                     _handle(mv._handle),
                     _buffer(std::move(mv._buffer))
                     _dirty(mv._dirty){

                    _mv.handle.id = -1;
                }
            private:
                cache_record( const cache_record& ) = delete;
                cache_record( cache& c, db_handle h )
                :_cache(c),_handle(h){}

                cache&              _cache;
                db_handle           _handle;
                std::vector<char>   _buffer;
                bool                _dirty = false;
        };

        template<typename L>
        cache_record& db_create( L&& constructor  ) {
            db_handle new_handle;
            intrinsic::db_create( rec.size, &new_handle );
            auto& r = _db_cache[new_handle.id] = cache_record(*this, new_handle).modify( std::forward<L>(constructor) );
            _dirty.push_back(&r);
        }

        cache_record& db_get( db_handle h ) {
            auto itr = _db_cache.find(h.id);
            if( itr != _db_cache.end() ) return itr->second;

            size_t s = db_get( h );

            if( s <= 0 ) {
                /// some kind of error happened, abort!
            }

            auto& r = _db_cache[h] = cache_record(*this, h);

            r._buffer.resize( s );
            if( auto err = intrinsic::db_get( h, r._buffer.data(), s ) ) { 
                /// some kind of error happened, abort!
            }
            return r;
        }
    private:
        vector<cache_record*>       _dirty;
        map<uint32_t, cache_record> _db_cache;
};

/**
 *  Can be serialized, and resets _record to nullptr when _handle changes
 */
class db_ptr {
    public:
        const char* data()const {
            return _get_record().data();
        }
        size_t      size()const {
            return _get_record().size();
        }

        template<typename L>
        void modify( L&& mod ) {
            return _get_record().modify( std::forward<L>(mod) );
        }

        db_handle handle()const { return _handle; }
    private:
        db_handle            _handle;
        cache::cache_record* _record = nullptr;
};

template<typename ViewType>
class view_ptr_impl : private db_ptr {
    public:

};

template<typename T>
struct tree_root {
    T value;

};


int main( int argc, char** argv ) {

    return 0;
}















