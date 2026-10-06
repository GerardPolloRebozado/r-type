project(RTypeEditor)

file(GLOB_RECURSE ASSETS_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        COMMAND AssetBundler ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        DEPENDS AssetBundler ${ASSETS_LIST}
        COMMENT "Bundling assets for Editor..."
)

file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
list(APPEND IMGUI_SOURCES ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

file(GLOB RLIMGUI_SOURCES ${rlImGui_SOURCE_DIR}/*.cpp)
add_library(rlImGui STATIC ${RLIMGUI_SOURCES})
target_include_directories(rlImGui PUBLIC ${rlImGui_SOURCE_DIR})
target_link_libraries(rlImGui PUBLIC imgui raylib)

set(SRC_FILES
        src/EditorConfig.cpp
        src/EditorLayer.cpp
        src/EditorStyle.cpp
        src/Panels/AssetsPanel.cpp
        src/Panels/ConsolePanel.cpp
        src/Panels/HierarchyPanel.cpp
        src/Panels/InspectorPanel.cpp
        src/Panels/PreviewPanel.cpp
        src/Panels/SceneViewPanel.cpp
        src/Panels/SettingsPanel.cpp
        src/Plugins/PositionPluginEditor.cpp
        src/Plugins/TextPluginEditor.cpp
        src/Plugins/SpritePluginEditor.cpp
        src/Serialization/SceneSerializer.cpp
)

set(INC_FILES
        include/EditorConfig.hpp
        include/EditorLayer.hpp
        include/EditorStyle.hpp
        include/Panels/AssetsPanel.hpp
        include/Panels/ConsolePanel.hpp
        include/Panels/HierarchyPanel.hpp
        include/Panels/IEditorPanel.hpp
        include/Panels/InspectorPanel.hpp
        include/Panels/PreviewPanel.hpp
        include/Panels/SceneViewPanel.hpp
        include/Panels/SettingsPanel.hpp
        include/PluginEditor.hpp
        include/Plugins/PositionPluginEditor.hpp
        include/Plugins/TextPluginEditor.hpp
        include/Plugins/SpritePluginEditor.hpp
        include/Serialization/SceneSerializer.hpp
)

add_executable(RTypeEditor main.cpp ${SRC_FILES} ${INC_FILES} ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp)
target_include_directories(RTypeEditor PRIVATE include ${CMAKE_CURRENT_BINARY_DIR})

# JSON Serialization for Scene saving
CPMAddPackage(
        NAME nlohmann_json
        GITHUB_REPOSITORY nlohmann/json
        VERSION 3.11.3
)

target_link_libraries(RTypeEditor PRIVATE Engine rlImGui imgui nlohmann_json::nlohmann_json)
