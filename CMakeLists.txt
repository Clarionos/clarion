project( clarion )

cmake_minimum_required(VERSION 3.16.3)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set( CMAKE_PROJECT_VERSION          0.0.0 )
set( CMAKE_PROJECT_VERSION_MAJOR    0 )
set( CMAKE_PROJECT_VERSION_MINOR    0 )
set( CMAKE_PROJECT_VERSION_PATCH    0 )

find_package( Boost REQUIRED )


IF(NOT "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
  set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR} )
  set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
  message(STATUS "Setting up OpenSSL root and include vars to ${OPENSSL_ROOT_DIR}, ${OPENSSL_INCLUDE_DIR}")
ENDIF()
if(APPLE AND UNIX AND "${OPENSSL_ROOT_DIR}" STREQUAL "")
   set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl@1.1")
endif()
find_package( OpenSSL REQUIRED )

add_subdirectory( libraries )
add_subdirectory( tests )
add_subdirectory( external )
