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

struct db_handle {
    uint32_t id;
};


/**
 *  Defines an global API that represents an abstract database that is able to
 *  create, fetch, and free records.
 */
class intrinsic {
    public:
        static intrinsic& instance() {
            static intrinsic i;
            return i;
        }

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

            _database.emplace_back( std::vector<char>(size) );

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
            return ok;
        }

        error_code db_free( db_handle h ) {
            if( h.id >= _database.size() ) return invalid_handle_param;
            _free_list.emplace_back( h.id, std::move(_database[h.id]) );
            return ok;
        }

    private:
        std::vector< std::pair<uint32_t, std::vector<char> > > _free_list;
        std::vector< std::vector<char> >                       _database;
};


/**
 *  This serves as a wasm-side cache of records already feteched from the intrinsic interface,
 *  it has a singleton interface and all fetches from the database should check with the cache instead
 *  of going to the intrinsic API. The cache will fetch from the database if not found.
 */
class cache {
    public:
        /**
         *  This object holds the one-true copy of the buffer for a given db handle.
         *  It is movable but not copyable. 
         */
        class cache_record {
            public:
                size_t         size()const                 { return _buffer.size();   }
                const char*    data()const                 { return _buffer.data();   } 
                bool           is_dirty()const             { return _dirty;           }
                db_handle      handle()const               { return _handle;          }

                void           resize( uint32_t s )        { _buffer.resize(s);       }
                char*          data()                      { return _buffer.data();   } 


                /**
                 *  @return a non-const reference to this after setting the dirty bit and notifying
                 *  the cache
                 */
                cache_record& modify()const  { 
                    if( not _dirty ) {
                        _dirty = true; 
                        _cache._dirty.push_back(this);
                    }
                    return *const_cast<cache_record*>(this); 
                }

                cache_record( cache_record&& mv )
                    :_cache(mv._cache),
                     _handle(mv._handle),
                     _buffer(std::move(mv._buffer)),
                     _dirty(mv._dirty){
                    mv._handle.id = -1;
                }

                cache_record& operator = ( cache_record&& mv )  {
                    /// TODO assert &_cache == &mv._cache...  this may be redundant 
                    /// since the cache is a global singleton, there may be no good
                    /// reason to store _cache on the record at all and we can save
                    /// some memory.
                    
                    if( &mv != this ) {
                        _handle = mv._handle;
                        mv._handle.id = -1;
                        _buffer = std::move(mv._buffer);
                        _dirty = mv._dirty;
                    }
                    return *this;
                }
            private:
                cache_record( const cache_record& ) = delete;
                cache_record( cache& c, db_handle h )
                :_cache(c),_handle(h){}

                cache&              _cache;
                db_handle           _handle;
                std::vector<char>   _buffer;
                mutable bool        _dirty = false;

                friend class cache;
        };

        cache_record& create( size_t new_size ) {
            db_handle new_handle;
            intrinsic::instance().db_create( new_size, &new_handle );
            auto& r = _db_cache.emplace( std::pair<uint32_t,cache_record>(
                          new_handle.id, 
                          cache_record(*this, new_handle)
                      ) ).first->second;
            r.modify(); // pushes it to cache
            return r;
        }

        const cache_record& get( db_handle h ) {
            auto itr = _db_cache.find(h.id);
            if( itr != _db_cache.end() ) return itr->second;

            size_t s = intrinsic::instance().db_get_size( h );

            if( s <= 0 ) {
                /// some kind of error happened, abort!
            }

            auto& r =  _db_cache.emplace( h.id, cache_record(*this, h) ).first->second;

            r._buffer.resize( s );
            if( auto err = intrinsic::instance().db_get( h, r._buffer.data(), s ) ) { 
                /// some kind of error happened, abort!
            }
            return r;
        }

        static cache& instance() {
            static cache c;
            return c;
        }
    private:
        std::vector<const cache_record*>  _dirty;
        std::map<uint32_t, cache_record>  _db_cache;
};


template<typename T>
struct db_ptr;

template<typename T>
struct db_ref {
    db_ref(const cache::cache_record& c):_cr(c){}

    const T* operator->()const {
        return reinterpret_cast<const T*>( _cr.data() );
    }

    T* operator->() {
        return reinterpret_cast<T*>( _cr.modify().data() );
    }

    operator db_ptr<T>()const { return db_ptr<T>(_cr.handle()); }
    operator db_handle()const { return _cr.handle(); }
    private:
    const cache::cache_record& _cr;
};

template<typename T>
struct db_ptr {
    db_ptr(){}
    db_ptr( db_handle h ):id(h.id){}

    uint32_t id = -1;

    bool operator !()const { return id == -1; }

    friend bool operator == ( const db_ptr<T>& a, const db_ptr<T>& b ) {
        return a.id == b.id;
    }
    friend bool operator != ( const db_ptr<T>& a, const db_ptr<T>& b ) {
        return a.id != b.id;
    }

    const db_ref<T> operator->()const {
        return cache::instance().get( {id} );
    }
    const db_ref<T> operator*()const {
        return cache::instance().get( {id} );
    }
};

template<typename T, typename... Args>
db_ref<T> make_db_ptr(Args&&... args) {
    auto& cr =  cache::instance().create( sizeof(T) );
    cr.resize( sizeof(T) );
    new (cr.data()) T( std::forward<Args>(args)... );
    return cr;
}



/**
 * This is the data that is actually stored in the database, it should
 * be trivially copyable.
 */
template<typename T>
struct node {
    static_assert( std::is_trivially_copyable<T>::value );

    template<typename... Args>
    node( Args&&... args ):data( std::forward<Args>(args)... ){}

    db_ptr<node<T>>   left;
    db_ptr<node<T>>   right;
    db_ptr<node<T>>   parent;
    int               height = 0;
    T                 data;
};

/**
 *  The underlying node<T> object only has db_ptr<> for left/right, which
 *  means that every time you dereference one of them you have to go to 
 *  the cache and do a map/hash table lookup. This would dramatically hurt
 *  the performance of the log(n) lookup required.  The node_cache object
 *  keeps a cache of the left/right db_ref that will only fectch if they
 *  have not yet been loaded from cache/db.
 *
 *  When performing tree operations you must modify both self->left/right and
 *  this->left/right or the cache will break its mirror to the underlying.
 */
template<typename T>
class node_cache : public std::enable_shared_from_this<node_cache<T>> {
    public:
        using node_cptr = std::shared_ptr<const node_cache<T>>;
        using node_db_ptr = db_ptr<node<T>>;

        node_cache( db_ref< node<T> > s ):self(s){}

        const T& value()const  { return self->data;   }
        int      height()const { return self->height; }

        const node_cptr& left()const {
            if( not _left and !!self->left ) {
                _left = std::make_shared<node_cache<T>>( *self->left );
            }
            return _left;
        }
        const node_cptr& right()const {
            if( not _right and !!self->right ) {
                _right = std::make_shared<node_cache<T>>( *self->right );
            }
            return _right;
        }
        const node_cptr& parent()const {
            if( not _parent and !!self->parent ) {
                _parent = std::make_shared<node_cache<T>>( *self->parent );
            }
            return _parent;
        }

        void set_parent( node_cptr new_parent )const {
            _parent = new_parent;

            node_db_ptr  new_parent_id = _parent ? _parent->ptr() : node_db_ptr();
            if( self->parent != new_parent_id ) {
               const_cast<db_ref<node<T>>&>(self)->parent = new_parent_id;
            }
        }

        void set_right( node_cptr new_right )const {
            _right = new_right;

            node_db_ptr  new_right_id = _right ? _right->ptr() : node_db_ptr();
            if( self->right != new_right_id ) {
               const_cast<db_ref<node<T>>&>(self)->right = new_right_id;
            }
            if( _right ) _right->set_parent( this->shared_from_this() );
        }

        void set_left( node_cptr new_left )const {
            _left = new_left;
            node_db_ptr  new_left_id = _left ? _left->ptr() : node_db_ptr();
            if( self->left != new_left_id ) {
               const_cast<db_ref<node<T>>&>(self)->left = new_left_id;
            }
            if( _left ) _left->set_parent( this->shared_from_this() );
        }

        void set_height( int h )const {
            if( self->height != h ) {
               const_cast<db_ref<node<T>>&>(self)->height = h;
            }
        }


        /*
        operator db_ref<node<T>>& () { return self; }
        operator const db_ref<node<T>>& ()const { return self; }
        operator db_ptr<node<T>>()const { return self; }
        operator db_handle()const       { return self; }
        db_handle handle()const         { return self; }
        */

        db_ptr<node<T>> ptr()const { return self; }
    private:

        db_ref<node<T> >  self;

        mutable node_cptr  _left;
        mutable node_cptr  _right;
        mutable node_cptr  _parent;
};




/**
 *  This is the datatype that stores the tree in the DB, it only
 *  stores the root.
 */
template<typename T>
struct tree {
    typedef db_ptr<node<T>> node_type;
    node_type root;
};

/**
 *  This is the interface that actually impliments the algorithms on
 *  the underlying memory.  
 */
template<typename T>
class tree_cache {
    using node_cptr = std::shared_ptr<const node_cache<T>>;

    public:

        /**
         *  Constructed with a reference to the tree database record,
         *  the tree is a record that contains a single db handle to
         *  the current root of the tree.  
         */
        tree_cache( db_ref< tree<T> > s ):self(s){}

        void insert( const T& v )const {
            set_root( insert( v, root() ) );
        }

        template<typename L>
        void visit( L&& v )const {
            visit( std::forward<L>(v), root() );
        }


        struct iterator {
            iterator(){}
            const T& operator*()const {
                return pos->value();
            }
            iterator& operator++() {
                if( not pos ) 
                    return *this;
                //std::cout << "start: " << **this << "\n";

                auto& right = pos->right();
                if( right ) {
                    pos = right;
                 //   std::cout << "    go right to: " << **this << "\n";
                    while( auto& left = pos->left() ) {
                        pos = left;
                  //      std::cout << "        go left to: " << **this << "\n";
                    }
                } else {
                   // std::cout << "    go to parent while parent right != me\n";
                    while( auto& parent = pos->parent() ) {
                        if( parent->right() == pos ) {
                            pos = parent;
                    //        std::cout << "        go to next parent: " << **this << "\n";
                        } else {
                     //       std::cout << "        go to parent: " << **this << "\n";
                            pos = parent;
                            return *this;
                        }
                    }
                //    std::cout << "    no more parents!  We are done\n";
                    pos = node_cptr();
                }
                return *this;
            }

            friend bool operator==( const iterator& a, const iterator& b) {
                return a.pos == b.pos;
            }
            friend bool operator!=( const iterator& a, const iterator& b ) {
                return a.pos != b.pos;
            }
            private:
                friend class tree_cache;
                iterator( const node_cptr& p ):pos(p){}
                node_cptr pos;
        };

        iterator lower_bound( const T& v )const {
            return lower_bound( v, root() );
        }
        iterator end()const { return iterator(node_cptr()); }

    private:
        int height( const node_cptr& t )const {
            return t ? t->height() : -1;
        }

        template<typename L>
        void visit( L&& v, const node_cptr& t )const {
            if( not t ) return;
            visit( std::forward<L>(v), t->left() );
            v( t->value() );
            visit( std::forward<L>(v), t->right() );
        }

        iterator lower_bound( const T& v, const node_cptr& t )const {
            if( not t ) return iterator();
            std::cout << " t->value: " << t->value()<<"\n";
            if( v < t->value() ) {
                auto& left = t->left();
                if( left ) 
                    return lower_bound( v, left );
            }
            else if( v > t->value() ) {
                auto& right = t->right();
                if( right ) 
                    return lower_bound( v, right );
            }
            return iterator(t);
        }

        node_cptr insert( const T& v, node_cptr t )const {
            if( not t ) {
                return make_node( v );
            } else if( v < t->value() ) {
                 t->set_left( insert( v, t->left() ) );    
                 auto tl = t->left();
                 auto tr = t->right();
                 if( height(tl) - height(tr) == 2 )
                 {
                    if( v < tl->value() ) {
                        t = single_right_rotate( t );
                    } else {
                        t = double_right_rotate( t );
                    }
                 }

            } else if( v > t->value() ) {
                 t->set_right( insert( v, t->right() ) );    
                 auto tl = t->left();
                 auto tr = t->right();
                 if( height(tr) - height(tl) == 2 )
                 {
                    if( v > tr->value() ) {
                        t = single_left_rotate( t );
                    } else {
                        t = double_left_rotate( t );
                    }
                 }
            }
            t->set_height( std::max( height( t->left() ), height( t->right() ) ) + 1);
            return t;
        }

        node_cptr single_right_rotate( const node_cptr& t )const {
            auto u = t->left();
            t->set_left( u->right() );
            u->set_right( t );
            t->set_height( std::max( height(t->left()), height(t->right()))+1 );
            u->set_height( std::max( height(u->left()), height(u->right()))+1 );
            return u;
        }
        node_cptr single_left_rotate( const node_cptr& t )const {
            auto u = t->right();
            t->set_right( u->left() );
            u->set_left( t );
            t->set_height( std::max( height( t->left()), height(t->right()))+1 );
            u->set_height( std::max( height( u->left()), height(u->right()))+1 );
            return u;
        }

        node_cptr double_left_rotate( const node_cptr& t )const {
            t->set_right( single_right_rotate( t->right() ) );
            return single_left_rotate( t );
        }

        node_cptr double_right_rotate( const node_cptr& t )const {
            t->set_left( single_left_rotate( t->left() ) );
            return single_right_rotate( t );
        }

        const node_cptr& root()const {
            if( not _root and !!self->root ) {
                set_root( std::make_shared<node_cache<T>>( *self->root ) );
            }
            return _root;
        }

        /**
         *  This function ensures the invariant that self.id == r->id, it
         *  is too easy change one without changing the other... therefore
         *  we use 'const' everywhere and this one function is given the
         *  right to cast it away and update the root.
         */
        void set_root( const node_cptr& r )const {
            const_cast<tree_cache*>(this)->set_root( r );
        }
        void set_root( const node_cptr& r ) {
            _root      = r;
            if( not _root ) {
                self->root = db_ptr<node<T>>(); /// null id
            } else {
                std::cout << "_root->ptr.id(): " << _root->ptr().id<<"  self->root.id: " 
                          << self->root.id << "  value: " << _root->value()<<"\n";
                /// TODO: make sure things really did change so we
                /// don't trigger the dirty bit
                //if( _root->handle().id != self->root ) {
                
                if( _root->ptr() != self->root ) {
                    std::cout << " current root: " << self->root.id ;
                    std::cout << " setting root: "<< _root->ptr().id  <<" \n";
                    self->root = _root->ptr(); //.id = _root->handle().id;
                }
                _root->set_parent( node_cptr() );
            }
        }

        node_cptr make_node( const T& v )const  {
            return std::make_shared<node_cache<T>>( make_db_ptr<node<T>>( v ) );
        }


        db_ref< tree<T> >     self;
        node_cptr             _root;
};


/*
template<typename T, typename ChildIndex>
struct base_index : {
    T value;
}

template<typename T, typename BaseIndex>
struct index : base_index<T, index> {
   index* left; 
   index* right; 
   index* parent; 
   int    height;
}
*/


int main( int, char** ){

 //   auto root = make_db_ptr<node<int>>( 5 );
    tree_cache<int>  tc( make_db_ptr<tree<int>>() );
    /*
    tc.insert( 6 );
    tc.insert( 8 );
    tc.insert( 9 );
    tc.insert( 2 );
    tc.insert( 41 );
    tc.insert( 42);
    tc.insert( 43);
    tc.insert( 44);
    tc.insert( 45);
    tc.insert( 46);
    */
    for( uint32_t i =0; i < 32; ++i )
        tc.insert( i );

    auto itr = tc.lower_bound( 10 );
    while( itr != tc.end() ) {
        std::cout << "value: " << *itr <<"\n";
        ++itr;
    }

    /*
    tc.visit( [&]( const auto& v ) {
        std::cout << v << "\n";
    });
    */

 //   auto itr = tr->lower_bound(5);
    
    //tr->insert( 5 );

//    std::cout << "root: " << root->data <<"\n";

//    clio::flat_ptr<node<int>> n = node<int>();

    return 0;
}
