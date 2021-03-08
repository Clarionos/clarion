set(BOOST_URL https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz)
set(BOOST_ARCHIVE ${CMAKE_CURRENT_BINARY_DIR}/boost_1_75_0.tar.gz)
set(BOOST_EXTRACTED ${CMAKE_CURRENT_BINARY_DIR}/boost_1_75_0)

if(NOT EXISTS ${BOOST_ARCHIVE})
    message("Downloading ${BOOST_URL}")
    file(DOWNLOAD ${BOOST_URL} ${BOOST_ARCHIVE}
       TIMEOUT 60
       SHOW_PROGRESS
       TLS_VERIFY ON)
endif()

if(NOT EXISTS ${BOOST_EXTRACTED})
    message("Extracting ${BOOST_ARCHIVE}")
    execute_process(
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND tar xf ${BOOST_ARCHIVE}
    )
endif()

add_library(boost INTERFACE)
target_include_directories(boost INTERFACE ${BOOST_EXTRACTED})
