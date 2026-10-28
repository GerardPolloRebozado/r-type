add_executable(Animation main.cpp)
target_include_directories(Animation PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Animation PRIVATE Engine)
target_compile_definitions(Animation PRIVATE ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
