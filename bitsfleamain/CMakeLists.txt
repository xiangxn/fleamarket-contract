
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/include/types.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/include/types.hpp)

add_contract(bitsfleamain bitsfleamain ${CMAKE_CURRENT_SOURCE_DIR}/src/bitsfleamain.cpp)
target_include_directories(bitsfleamain.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(bitsfleamain.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
