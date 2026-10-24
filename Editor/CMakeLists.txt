project(RTypeEditor)

file(GLOB_RECURSE ASSETS_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

set(ASSET_BUNDLER_EXECUTABLE "${CMAKE_BINARY_DIR}/AssetBundler")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        COMMAND ${ASSET_BUNDLER_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        DEPENDS AssetBundler ${ASSETS_LIST}
        COMMENT "Bundling assets for Editor..."
)



add_library(imgui_textedit STATIC
        third_party/ImGuiColorTextEdit/TextEditor.cpp
)
target_include_directories(imgui_textedit PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ImGuiColorTextEdit
)
target_link_libraries(imgui_textedit PUBLIC imgui)

set(SRC_FILES
        src/EditorLayer.cpp
        src/EditorStyle.cpp
        src/EntitySubtree.cpp
        src/Panels/AssetsPanel.cpp
        src/Panels/ConnectionsPanel.cpp
        src/Panels/ConsolePanel.cpp
        src/Panels/HierarchyPanel.cpp
        src/Panels/InspectorPanel.cpp
        src/Panels/PreviewPanel.cpp
        src/Panels/SceneViewPanel.cpp
        src/Panels/ScriptEditorPanel.cpp
        src/Panels/SettingsPanel.cpp
        src/Panels/InputMappingPanel.cpp
        src/Panels/StatePanel.cpp
        src/Gallery/Gallery.cpp
        src/Plugins/PositionPluginEditor.cpp
        src/Plugins/TextPluginEditor.cpp
        src/Plugins/SpritePluginEditor.cpp
        src/Plugins/NetworkComponentPluginEditor.cpp
        src/Plugins/ShapePluginEditor.cpp
        src/Plugins/RotationPluginEditor.cpp
        src/Plugins/OpacityPluginEditor.cpp
        src/Plugins/ColliderPluginEditor.cpp
        src/Plugins/ConnectionsPluginEditor.cpp
        src/Plugins/ScriptPluginEditor.cpp
        src/Plugins/CameraPluginEditor.cpp
        src/Plugins/AudioPluginEditor.cpp
        src/Rendering/SceneRenderer.cpp
        src/ProjectSystemsBuild.cpp
        src/Scaffold.cpp
        src/Scripting/LuaLanguage.cpp
        src/Scripting/LuaSyntax.cpp
        src/Utils/FileDialog.cpp
        src/Utils/Multiplatform.cpp
)

set(INC_FILES
        include/EditorConfig.hpp
        include/EditorLayer.hpp
        include/EditorStyle.hpp
        include/EntitySubtree.hpp
        include/Panels/AssetsPanel.hpp
        include/Panels/ConnectionsPanel.hpp
        include/Panels/ConsolePanel.hpp
        include/Panels/HierarchyPanel.hpp
        include/Panels/IEditorPanel.hpp
        include/Panels/InspectorPanel.hpp
        include/Panels/PreviewPanel.hpp
        include/Panels/SceneViewPanel.hpp
        include/Panels/ScriptEditorPanel.hpp
        include/Panels/SettingsPanel.hpp
        include/Panels/InputMappingPanel.hpp
        include/Panels/StatePanel.hpp
        include/Gallery.hpp
        include/FileDialog.hpp
        include/PluginEditor.hpp
        include/Plugins/PositionPluginEditor.hpp
        include/Plugins/TextPluginEditor.hpp
        include/Plugins/SpritePluginEditor.hpp
        include/Plugins/NetworkComponentPluginEditor.hpp
        include/Plugins/ShapePluginEditor.hpp
        include/Plugins/RotationPluginEditor.hpp
        include/Plugins/OpacityPluginEditor.hpp
        include/MultiplatformUtils.hpp
        include/Plugins/ColliderPluginEditor.hpp
        include/Plugins/ConnectionsPluginEditor.hpp
        include/Plugins/CameraPluginEditor.hpp
        include/Plugins/AudioPluginEditor.hpp
        include/Rendering/SceneRenderer.hpp
        include/GalleryReflect.hpp
        include/ProjectSystemsBuild.hpp
        include/Scaffold.hpp
        include/ScriptEditorService.hpp
        include/Scripting/LuaLanguage.hpp
        include/Scripting/LuaSyntax.hpp
        include/Compat/WindowsApi.hpp
)

add_executable(RTypeEditor main.cpp ${SRC_FILES} ${INC_FILES} ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp)
target_include_directories(RTypeEditor PRIVATE include ${CMAKE_CURRENT_BINARY_DIR}
        third_party/ImGuiColorTextEdit)
target_compile_definitions(RTypeEditor PRIVATE "RTYPE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")



# Portable File Dialogs
set(PFD_URL "https://raw.githubusercontent.com/samhocevar/portable-file-dialogs/master/portable-file-dialogs.h")
set(PFD_DEST "${CMAKE_CURRENT_BINARY_DIR}/include/portable-file-dialogs.h")
if (NOT EXISTS "${PFD_DEST}")
    message(STATUS "Downloading portable-file-dialogs.h...")
    file(DOWNLOAD "${PFD_URL}" "${PFD_DEST}")
endif ()
target_include_directories(RTypeEditor PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")

target_link_libraries(RTypeEditor PRIVATE Engine Serialization rlImGui imgui_textedit imgui glaze::glaze)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    target_compile_options(RTypeEditor PRIVATE -s ASYNCIFY)
    target_link_options(RTypeEditor PRIVATE -s ASYNCIFY)
endif()

include(BakeEditorToolchain)
bake_editor_toolchain(RTypeEditor)
