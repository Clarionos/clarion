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

find_package(Boost REQUIRED COMPONENTS filesystem program_options)
find_package(Git)

IF(NOT "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
  set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR} )
  set(OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include)
  message(STATUS "Setting up OpenSSL root and include vars to ${OPENSSL_ROOT_DIR}, ${OPENSSL_INCLUDE_DIR}")
ENDIF()
if(APPLE AND UNIX AND "${OPENSSL_ROOT_DIR}" STREQUAL "")
   set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl@1.1")
endif()
find_package( OpenSSL REQUIRED )


if( APPLE )
  # Apple Specific Options Here
  message( STATUS "Configuring AppBase on OS X" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-conversion -Wno-deprecated-declarations" )
else( APPLE )
  # Linux Specific Options Here
  message( STATUS "Configuring AppBase on Linux" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
  if ( FULL_STATIC_BUILD )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
  endif ( FULL_STATIC_BUILD )
endif( APPLE )



add_subdirectory( libraries )
add_subdirectory( tests )
add_subdirectory( external )
