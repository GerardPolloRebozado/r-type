cmake_minimum_required(VERSION 3.28)
project(UI)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(UI STATIC
        src/UiCatalog.cpp
        src/UiPlugin.cpp
        src/UiSystem.cpp
        src/UiRuntime.cpp
        src/LuaAPI/UiAPI.cpp
        third_party/sol2_ImGui_Bindings/sol_ImGui.cpp
)

target_include_directories(UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(UI PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sol2_ImGui_Bindings
)

target_link_libraries(UI PUBLIC Engine rlImGui imgui)

# sol2 + ImGui bindings generate >32k COFF sections; MinGW's assembler needs big-obj.
if (MINGW)
    target_compile_options(UI PRIVATE -Wa,-mbig-obj)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # On WASM the absolute host path doesn't exist in the emscripten virtual
    # filesystem. Templates are embedded at UI/lua/templates@UI/lua/templates
    # in the root CMakeLists.txt, so use a VFS-relative path instead.
    target_compile_definitions(UI PRIVATE
            "UI_TEMPLATES_DIR=\"UI/lua/templates\""
            IMGUI_DOCKING
    )
else ()
    target_compile_definitions(UI PRIVATE
            "UI_TEMPLATES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/lua/templates\""
            IMGUI_DOCKING
    )
endif ()
