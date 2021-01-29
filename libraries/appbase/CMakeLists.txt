list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

include( InstallDirectoryPermissions )
include( GNUInstallDirs )

file(GLOB HEADERS "include/appbase/*.hpp")

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

if(CMAKE_CXX_STANDARD EQUAL 98 OR CMAKE_CXX_STANDARD LESS 14)
   message(FATAL_ERROR "appbase requires c++14 or newer")
elseif(NOT CMAKE_CXX_STANDARD)
   set(CMAKE_CXX_STANDARD 14)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads)

add_library( appbase
             application.cpp
             version.cpp
             ${HEADERS}
           )

target_link_libraries( appbase Boost::program_options Boost::filesystem Threads::Threads)

target_include_directories( appbase
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties( appbase PROPERTIES PUBLIC_HEADER "${HEADERS}" )


if(EXISTS ${CMAKE_SOURCE_DIR}/.git AND GIT_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/version.cmake @ONLY)
  add_custom_target(appbase_version ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/version.cmake BYPRODUCTS version.cpp)
  add_dependencies(appbase appbase_version)
else()
  set(VERSION_STRING "Unknown")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.cpp @ONLY ESCAPE_QUOTES)
endif()

set(CPACK_PACKAGING_INSTALL_PREFIX /)

install( TARGETS
   appbase

   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/appbase
)
install_directory_permissions( DIRECTORY ${CMAKE_INSTALL_FULL_INCLUDEDIR}/appbase )

