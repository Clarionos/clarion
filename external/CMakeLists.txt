include(ExternalProject)

#add_subdirectory( quill )
add_subdirectory( Catch2 )
add_subdirectory( secp256k1 )

add_library( rapidjson INTERFACE )
target_include_directories( rapidjson INTERFACE rapidjson/include )

ExternalProject_Add(fmt
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fmt
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/../installed
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DFMT_DOC=off
        -DFMT_TEST=off
)

ExternalProject_Add(googletest
    URL https://github.com/google/googletest/archive/release-1.8.0.tar.gz
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/../installed
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

ExternalProject_Add(folly
    DEPENDS fmt
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/folly
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/../installed
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
)

ExternalProject_Add(fizz
    DEPENDS folly googletest
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fizz/fizz
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/../installed
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
)

ExternalProject_Add(mvfst
    DEPENDS folly fizz
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mvfst
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/../installed
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_TESTS=OFF
        -DBUILD_EXAMPLES=OFF
)
