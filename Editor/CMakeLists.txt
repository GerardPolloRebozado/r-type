project(RTypeEditor)

file(GLOB_RECURSE ASSETS_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        COMMAND AssetBundler ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        DEPENDS AssetBundler ${ASSETS_LIST}
        COMMENT "Bundling assets for Editor..."
)



set(SRC_FILES
        src/EditorLayer.cpp
        src/EditorStyle.cpp
        src/Panels/AssetsPanel.cpp
        src/Panels/ConsolePanel.cpp
        src/Panels/HierarchyPanel.cpp
        src/Panels/InspectorPanel.cpp
        src/Panels/PreviewPanel.cpp
        src/Panels/SceneViewPanel.cpp
        src/Panels/SettingsPanel.cpp
        src/Panels/InputMappingPanel.cpp
        src/Panels/StatePanel.cpp
        src/Gallery/Gallery.cpp
        src/Plugins/PositionPluginEditor.cpp
        src/Plugins/TextPluginEditor.cpp
        src/Plugins/SpritePluginEditor.cpp
        src/Plugins/ShapePluginEditor.cpp
        src/Plugins/RotationPluginEditor.cpp
        src/Plugins/ColliderPluginEditor.cpp
        src/Plugins/ScriptPluginEditor.cpp
        src/Plugins/CameraPluginEditor.cpp
        src/Rendering/SceneRenderer.cpp
        src/Utils/Multiplatform.cpp
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
        include/Panels/InputMappingPanel.hpp
        include/Panels/StatePanel.hpp
        include/Gallery.hpp
        include/PluginEditor.hpp
        include/Plugins/PositionPluginEditor.hpp
        include/Plugins/TextPluginEditor.hpp
        include/Plugins/SpritePluginEditor.hpp
        include/Plugins/ShapePluginEditor.hpp
        include/Plugins/RotationPluginEditor.hpp
        include/MultiplatformUtils.hpp
        include/Plugins/ColliderPluginEditor.hpp
        include/Plugins/CameraPluginEditor.hpp
        include/Rendering/SceneRenderer.hpp
)

add_executable(RTypeEditor main.cpp ${SRC_FILES} ${INC_FILES} ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp)
target_include_directories(RTypeEditor PRIVATE include ${CMAKE_CURRENT_BINARY_DIR})



# Portable File Dialogs
set(PFD_URL "https://raw.githubusercontent.com/samhocevar/portable-file-dialogs/master/portable-file-dialogs.h")
set(PFD_DEST "${CMAKE_CURRENT_BINARY_DIR}/include/portable-file-dialogs.h")
if (NOT EXISTS "${PFD_DEST}")
    message(STATUS "Downloading portable-file-dialogs.h...")
    file(DOWNLOAD "${PFD_URL}" "${PFD_DEST}")
endif ()
target_include_directories(RTypeEditor PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")

target_link_libraries(RTypeEditor PRIVATE Engine Serialization rlImGui imgui nlohmann_json::nlohmann_json)
