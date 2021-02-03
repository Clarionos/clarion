#SET( DEFAULT_HEADER_INSTALL_DIR usr/include/${target} )
#SET( DEFAULT_LIBRARY_INSTALL_DIR usr/lib )
#SET( DEFAULT_EXECUTABLE_INSTALL_DIR usr/bin )
SET( BUILD_SHARED_LIBS NO )
SET( ECC_IMPL secp256k1 CACHE STRING "secp256k1 or openssl or mixed" )

set(platformBitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(platformBitness 64)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

#include( GNUInstallDirs )
#include( SetupTargetMacros )

if(CMAKE_CXX_STANDARD EQUAL 98 OR CMAKE_CXX_STANDARD LESS 17)
   message(FATAL_ERROR "fc requires c++17 or newer")
elseif(NOT CMAKE_CXX_STANDARD)
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

#add_subdirectory( secp256k1 )

IF( ECC_IMPL STREQUAL openssl )
  SET( ECC_REST src/crypto/elliptic_impl_pub.cpp )
ELSE( ECC_IMPL STREQUAL openssl )
  SET( ECC_LIB secp256k1 )
  IF( ECC_IMPL STREQUAL mixed )
    SET( ECC_REST src/crypto/elliptic_impl_priv.cpp src/crypto/elliptic_impl_pub.cpp )
  ELSE( ECC_IMPL STREQUAL mixed )
    SET( ECC_REST src/crypto/elliptic_impl_priv.cpp )
  ENDIF( ECC_IMPL STREQUAL mixed )
ENDIF( ECC_IMPL STREQUAL openssl )


set( fc_sources
     src/uint128.cpp
     src/real128.cpp
     src/variant.cpp
     src/exception.cpp
     src/variant_object.cpp
     src/string.cpp
     src/time.cpp
     src/utf8.cpp
     src/io/datastream.cpp
     src/io/json.cpp
     src/io/varint.cpp
     src/io/fstream.cpp
     src/io/console.cpp
     src/filesystem.cpp
     src/interprocess/file_mapping.cpp
     src/interprocess/mmap_struct.cpp
     src/log/log_message.cpp
     src/log/logger.cpp
     src/log/appender.cpp
     src/log/console_appender.cpp
     src/log/logger_config.cpp
     src/crypto/_digest_common.cpp
     src/crypto/openssl.cpp
     src/crypto/aes.cpp
     src/crypto/crc.cpp
     src/crypto/city.cpp
     src/crypto/base36.cpp
     src/crypto/base58.cpp
     src/crypto/base64.cpp
     src/crypto/bigint.cpp
     src/crypto/hex.cpp
     src/crypto/sha1.cpp
     src/crypto/ripemd160.cpp
     src/crypto/sha256.cpp
     src/crypto/sha224.cpp
     src/crypto/sha512.cpp
     src/crypto/dh.cpp
     src/crypto/blowfish.cpp
     src/crypto/elliptic_common.cpp
     ${ECC_REST}
     src/crypto/elliptic_${ECC_IMPL}.cpp
     src/crypto/elliptic_r1.cpp
     src/crypto/elliptic_webauthn.cpp
     src/crypto/rand.cpp
     src/crypto/public_key.cpp
     src/crypto/private_key.cpp
     src/crypto/signature.cpp
     src/network/ip.cpp
     src/network/platform_root_ca.cpp
     src/network/resolve.cpp
     src/network/url.cpp
     src/network/http/http_client.cpp
     src/compress/smaz.cpp
     src/compress/zlib.cpp
     )

file( GLOB_RECURSE fc_headers ${CMAKE_CURRENT_SOURCE_DIR} *.hpp *.h )

set( sources
  ${fc_sources}
)

list(APPEND sources ${fc_headers})
add_library( fc STATIC ${fc_sources} )


if( ZLIB_FOUND )
  MESSAGE( STATUS "zlib found" )
  add_definitions( -DHAS_ZLIB )
else()
  MESSAGE( STATUS "zlib not found" )
  set( ZLIB_LIBRARIES "" )
endif( ZLIB_FOUND )

# This will become unnecessary once we update to websocketpp which fixes upstream issue #395
add_definitions(-DWEBSOCKETPP_STRICT_MASKING -DBOOST_ASIO_HAS_STD_CHRONO)

# try and make this very clear that this json parser is intended only for webauthn parsing..
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.11.0)
   set_source_files_properties(src/crypto/elliptic_webauthn.cpp PROPERTIES INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include/fc/crypto/webauthn_json/include)
else()
   target_include_directories(fc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/fc/crypto/webauthn_json/include)
endif()

IF(NOT WIN32)
  set(LINK_USR_LOCAL_LIB -L/usr/local/lib)
ENDIF()


IF(APPLE)
  find_library(security_framework Security)
  find_library(corefoundation_framework CoreFoundation)
ENDIF()

target_include_directories( fc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} )
target_link_libraries( fc PUBLIC ${LINK_USR_LOCAL_LIB}
                                 Boost::date_time Boost::filesystem Boost::chrono Boost::iostreams 
                                 ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} ${RPCRT4} ${CMAKE_DL_LIBS} ${readline_libraries} ${ECC_LIB} ${security_framework} ${corefoundation_framework} )

IF(WIN32)
  target_link_libraries( fc PUBLIC ws2_32 mswsock userenv )
ENDIF()


#SET(OPENSSL_CONF_TARGET )
#IF(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
#  SET (OPENSSL_CONF_TARGET ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#ELSE()
#  SET (OPENSSL_CONF_TARGET ${CMAKE_CURRENT_BINARY_DIR})
#ENDIF()

IF(NOT DEFINED SKIP_FC_TESTS)
  add_subdirectory( test )
ENDIF()

#install(TARGETS fc
#   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
#   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})
#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/fc DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})

