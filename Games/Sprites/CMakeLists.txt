add_executable(Sprites main.cpp)
target_link_libraries(Sprites PRIVATE Engine)
target_compile_definitions(Sprites PRIVATE ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
