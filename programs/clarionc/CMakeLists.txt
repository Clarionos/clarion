add_executable( clariond main.cpp 
#                websock/shared_state.cpp 
#                websock/listener.cpp 
#                websock/websocket_session.cpp 
#                websock/http_session.cpp 
                )
target_include_directories( clariond PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/websock )

target_link_libraries( clariond 
        PRIVATE appbase Boost::coroutine
        PRIVATE -Wl,${whole_archive_flag} clarion-net -Wl,${no_whole_archive_flag}
)
