add_executable(a.wasm a.cpp)

target_link_options(a.wasm PUBLIC -Wl,--export-table)
