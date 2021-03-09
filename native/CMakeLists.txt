cmake_minimum_required(VERSION 3.16.3)
cmake_policy(VERSION 3.16)
project(native)
enable_testing()

set(IS_NATIVE YES)
set(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
set(ROOT_BINARY_DIR ${CMAKE_BINARY_DIR}/..)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(native_test N)
    add_test(
        NAME n-${N}
        WORKING_DIRECTORY ${ROOT_BINARY_DIR}
        COMMAND ./${N}
    )
endfunction()

function(wasm_test N)
endfunction()

function(native_and_wasm_test N)
    native_test(${N})
endfunction()

find_package(Boost REQUIRED COMPONENTS filesystem program_options date_time chrono iostreams coroutine)
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

  # As of 10.10 yosemite, the OpenSSL static libraries shipped with os x have a dependency
  # on zlib, so any time you link in openssl you also need to link zlib.  .  We really want to detect whether openssl was configured with the --no-zlib
  # option or not when it was built, but that's difficult to do in practice, so we
  # just always try to link it in on mac.
  find_package( ZLIB REQUIRED )
  find_library(security_framework Security)
  find_library(corefoundation_framework CoreFoundation)


  message( STATUS "Configuring AppBase on OS X" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-conversion -Wno-deprecated-declarations" )
else( APPLE )
  find_package( ZLIB )

  # Linux Specific Options Here
  message( STATUS "Configuring AppBase on Linux" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
  if ( FULL_STATIC_BUILD )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
  endif ( FULL_STATIC_BUILD )
endif( APPLE )

if(UNIX)
  if(APPLE)
    set(whole_archive_flag "-force_load")
    set(no_whole_archive_flag "")
    set(build_id_flag "")
  else()
    set(whole_archive_flag "--whole-archive")
    set(no_whole_archive_flag "--no-whole-archive")
    set(build_id_flag "--build-id")
  endif()
else()
  set(whole_archive_flag "--whole-archive")
  set(no_whole_archive_flag "--no-whole-archive")
  set(build_id_flag "")
endif()


add_subdirectory( ../libraries libraries )
add_subdirectory( ../programs programs )
add_subdirectory( ../tests tests )
add_subdirectory( ../external external )
