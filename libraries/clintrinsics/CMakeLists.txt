add_library(clintrinsics)

target_include_directories(clintrinsics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(clintrinsics INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clintrinsics/basic.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clintrinsics/coroutines.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clintrinsics/database.hpp
)

target_sources(clintrinsics PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_args_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_args_sizes_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_clock_time_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_environ_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_environ_sizes_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_close.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_fdstat_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_fdstat_set_flags.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_prestat_dir_name.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_prestat_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_read.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_seek.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_write.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_path_open.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_proc_exit.cpp
)

target_link_options(clintrinsics INTERFACE -Wl,--export-table)
