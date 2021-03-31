add_contract(eden eden
   ${CMAKE_CURRENT_SOURCE_DIR}/src/actions/hi.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/actions/transfer_handler.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/src/members.cpp
)

target_include_directories(eden
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(eden
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/eden.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/eden.contracts.md @ONLY )

# target_compile_options( eden PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
