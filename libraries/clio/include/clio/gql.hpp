#pragma once
#include <clio/reflect.hpp>
#include <clio/name.hpp>

#include <variant>
#include <string>

namespace clio {

    /**
     *  The concept of our graph-ql like interface is that a graph-ql query is
     *  parsed and converted into a gql query object and then flattened into something
     *  that requires no unpacking inorder to dispatch. This means converting gql strings
     *  into base32 integer representations and/or hashes if he name does not fit in
     *  base32.
     *
     *  We use base32 (eg eosio::name) based 64bit integers as the key values because they
     *  map to swtich(key) statements for high performance dispatch by our resolvers.
     */
    namespace gql {

        struct null_t{};
        struct entry;
        struct scalar;
        using  object = std::vector<entry>;
        using  array  = std::vector<scalar>;


        /**
         *  The scalar is the base value type, it is effectively a JSON object that uses
         *  base32 names for object keys and supports int64_t and uint64_t types in addition
         *  to double, string, object, array, and null. 
         *
         *  In principle you can convert from JSON to Scalar without any schema, and if you know
         *  the original keys for your objects, you can query the scalar for the value. In rare
         *  circumstances two keys may result in a hash collision, this should be detected at compile
         *  time and can be resolved by changing a field name.
         *
         *  The flat view of a scalar is the same as a variant:
         *  [char_type][uint64_value] inline.  If the type is a string,object,or array then the
         *  value field is a offset_ptr<>
         *   
         */
        class scalar {
            public:
               scalar(){} 

               template<typename T>
               scalar( const T& v ):value(v){}

               using value_type = std::variant<null_t,double,int64_t,uint64_t,std::string,object,array>;

               value_type value;
        };
        using scalar_value = scalar::value_type;
        CLIO_REFLECT_TYPENAME_CUSTOM( null_t, null )
        CLIO_REFLECT_TYPENAME_CUSTOM( scalar_value, scalar )

        /**
         *  The key/value pair of an object
         */
        struct entry {
            uint64_t key;
            scalar   value;
        };


        struct query;
        struct query_filter;

        /**
         *  Defines what fields to select from the result of a query,
         *  if type is non-zero then the filter only applies if the type
         *  of the return value matches the variant type.
         */
        struct query_filter {
            std::vector<query> filter;
            /// the null type matches everything
            uint64_t           type = 0;
        };

        struct query {
            uint64_t                  key;
            uint64_t                  alias;
            object                    args;
            /// for each type... define the filter...
            std::vector<query_filter> filter;
        };
        CLIO_REFLECT( scalar, value )
        CLIO_REFLECT( entry, key, value )
        CLIO_REFLECT( query_filter, filter, type )
        CLIO_REFLECT( query, key, alias, args, filter )

    } // namespace graphql

} // namespace clio
