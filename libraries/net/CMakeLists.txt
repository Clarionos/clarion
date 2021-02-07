set( sources
      net_plugin.cpp
      listener.cpp
      shared_state.cpp
      http_session.cpp
      websocket_session.cpp
)

set( headers
     include/clarion/net/net_plugin.hpp
     include/clarion/net/beast.hpp
     include/clarion/net/http_session.hpp
     include/clarion/net/websocket_session.hpp
     include/clarion/net/shared_state.hpp
     include/clarion/net/net.hpp
     include/clarion/net/beast.hpp
     include/clarion/net/listener.hpp
   )

add_library( clarion-net STATIC ${sources} )
target_include_directories( clarion-net PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
target_link_libraries( clarion-net appbase Boost::coroutine )

