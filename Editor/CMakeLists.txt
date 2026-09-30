file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

file(GLOB RLIMGUI_SOURCES ${rlImGui_SOURCE_DIR}/*.cpp)
add_library(rlImGui STATIC ${RLIMGUI_SOURCES})
target_include_directories(rlImGui PUBLIC ${rlImGui_SOURCE_DIR})
target_link_libraries(rlImGui PUBLIC imgui raylib)

file(GLOB_RECURSE EDITOR_SOURCES "src/*.cpp")
add_executable(RTypeEditor main.cpp ${EDITOR_SOURCES})
target_include_directories(RTypeEditor PRIVATE include)

# JSON Serialization for Scene saving
CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.11.3
)

target_link_libraries(RTypeEditor PRIVATE Engine rlImGui imgui nlohmann_json::nlohmann_json)
