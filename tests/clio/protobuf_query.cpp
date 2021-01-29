#include <catch2/catch.hpp>
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


struct service {
    int add( int a, int b )const { 
        std::cout << a << " + " << b <<"\n";
        return a+ b; 
    }
    int sub( int a, int b ) { 
        std::cout << a << " - " << b <<"\n";
        return a - b; 
    }
};

CLIO_REFLECT( service, add, sub )


TEST_CASE( "try calling methods via protobuf", "[protobuf]" ) 
{
    clio::schema service_schema;
    service_schema.generate<service>();
    service_schema.generate<clio::protobuf::query>();
    std::cout << "schema: " << format_json( service_schema )<<"\n";


    clio::reflect<service>::proxy<clio::protobuf::query_proxy> query;
    query.add( 2, 3 );
    query.add( 8, 5 );
    query.sub( 2, 3 );

    std::cout << clio::format_json( query->q ) <<"\n";
    auto protobuf_query = clio::to_protobuf( query->q );

    clio::input_stream  protobuf_query_in( protobuf_query.data(), protobuf_query.size() );
    std::string json_query;
    clio::string_stream json_query_out(json_query);

    clio::translate_protobuf_to_json( service_schema, "query", protobuf_query_in, json_query_out );


    std::cout << "pbuf query: " << format_json( clio::bytes{protobuf_query} )<<"\n";
    std::cout << "json pbuf query: " << json_query<<"\n";
    std::cout << "json query: " << clio::format_json( clio::protobuf::to_json_query<service>( query->q ) ) <<"\n";

    service s;
    auto result = clio::protobuf::dispatch( s, query->q );

    std::cout << clio::format_json( result ) <<"\n";

    auto protobuf_result = clio::to_protobuf( result );
    std::string json_result;

    clio::input_stream  protobuf_in( protobuf_result.data(), protobuf_result.size() );
    clio::string_stream json_out(json_result);

    clio::translate_protobuf_to_json( service_schema, "service", protobuf_in, json_out );

    std::cout << "json result: " << json_result <<"\n";

}
