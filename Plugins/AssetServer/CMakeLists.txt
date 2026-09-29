find_package(raylib CONFIG REQUIRED)
find_path(RAYLIB_CPP_INCLUDE_DIRS "raylib-cpp.hpp")

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_library(AssetServer STATIC ${SRC_FILES}
        include/AssetServer.hpp
        include/Asset.hpp)
target_include_directories(PluginSound PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${RAYLIB_CPP_INCLUDE_DIRS}
        ${RAYLIB_CPP_INCLUDE_DIRS}/raylib-cpp
)
target_link_libraries(PluginSound
        PUBLIC Engine::Headers
        PRIVATE raylib
)
