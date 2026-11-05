add_executable(Menu main.cpp)
target_link_libraries(Menu PRIVATE Engine UI)
target_compile_definitions(Menu PRIVATE "MENU_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
