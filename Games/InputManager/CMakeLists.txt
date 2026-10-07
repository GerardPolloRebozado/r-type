add_executable(InputManager main.cpp)
target_include_directories(InputManager PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(InputManager PRIVATE Engine)
