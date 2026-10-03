add_executable(Text main.cpp)
target_include_directories(Text PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Text PRIVATE Engine)
