add_library(clintrinsics)

target_include_directories(clintrinsics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(clintrinsics INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clintrinsics/basic.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clintrinsics/coroutines.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/clintrinsics/database.hpp
)

target_sources(clintrinsics PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_proc_exit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_fdstat_get.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_write.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_seek.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasi_polyfill/__wasi_fd_close.cpp
)

target_link_options(clintrinsics INTERFACE -Wl,--export-table)
