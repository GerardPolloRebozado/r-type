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

target_compile_definitions(UI PRIVATE
        "UI_TEMPLATES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/lua/templates\""
        IMGUI_DOCKING
)
