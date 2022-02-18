add_library( clio INTERFACE )
target_link_libraries( clio INTERFACE rapidjson )
target_include_directories( clio INTERFACE include ${Boost_INCLUDE_DIRS} )
add_compile_options( -Wall -Wstrict-aliasing )

target_sources( clio INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/fpconv.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/powers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/compress.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/error.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/reflect.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/schema.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/translator.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/stream.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_key.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/tuple.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/flatbuf.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_json.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_json/map.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_json/varint.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/json/any.hpp


        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_bin.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_bin/map.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_bin/set.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_bin/list.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_bin/deque.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_bin/varint.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/from_bin.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/from_json.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/varint.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/from_bin/varint.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/from_json/varint.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/bytes.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/bytes/from_json.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/bytes/to_json.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/bytes/to_bin.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/from_protobuf.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/to_protobuf.hpp

        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/protobuf/any.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/protobuf/json.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/protobuf/query.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/clio/protobuf/schema.hpp
)
