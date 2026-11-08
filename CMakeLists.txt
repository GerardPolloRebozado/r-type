cmake_minimum_required(VERSION 3.28)
set(CMAKE_POLICY_DEFAULT_CMP0169 OLD)
project(G-ING-500-BAR-5-1-rtype-1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif ()
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE BOOL "Suppress developer warnings" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(BundleMingwRuntime)
bundle_mingw_runtime()

set(CPM_DOWNLOAD_VERSION 0.40.0)
set(CPM_HASH_SUM "7b354f3a5976c4626c876850c93944e52c83ec59a159ae5de5be7983f0e17a2a")

if (NOT CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm_cache" CACHE PATH "CPM source cache")
endif ()

set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

if (NOT EXISTS "${CPM_DOWNLOAD_LOCATION}")
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}...")
    file(DOWNLOAD
            "https://raw.githubusercontent.com/cpm-cmake/CPM.cmake/v${CPM_DOWNLOAD_VERSION}/cmake/CPM.cmake"
            "${CPM_DOWNLOAD_LOCATION}"
            EXPECTED_HASH SHA256=${CPM_HASH_SUM}
    )
endif ()
include(${CPM_DOWNLOAD_LOCATION})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

CPMAddPackage(
        NAME raylib
        GITHUB_REPOSITORY raysan5/raylib
        GIT_TAG 6.2
        OPTIONS
        "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
        NAME raylib-cpp
        GITHUB_REPOSITORY RobLoach/raylib-cpp
        GIT_TAG v6.0.3
)

# NOTE: pinned to a fixed docking commit (Dear ImGui 1.93.1). The floating
# `docking` branch resolves to a different snapshot per machine/fetch date,
# which broke the build (tab color renames in 1.92+: TabActive -> TabSelected,
# TabUnfocused -> TabDimmed, TabUnfocusedActive -> TabDimmedSelected).
CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG d88f238d1d363ef387382b2e29ad12cf96f826ac
)
CPMAddPackage(
        NAME rlImGui
        GITHUB_REPOSITORY raylib-extras/rlImGui
        GIT_TAG main
)

CPMAddPackage(
        NAME enet
        GITHUB_REPOSITORY zpl-c/enet
        GIT_TAG v2.7.0
)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

set(RLIMGUI_SOURCES
        ${rlImGui_SOURCE_DIR}/rlImGui.cpp
)
add_library(rlImGui STATIC ${RLIMGUI_SOURCES})
target_include_directories(rlImGui PUBLIC ${rlImGui_SOURCE_DIR})
target_link_libraries(rlImGui PUBLIC imgui raylib)

CPMAddPackage(
        NAME glaze
        GITHUB_REPOSITORY stephenberry/glaze
        GIT_TAG v9.0.0
)

CPMAddPackage(
        NAME sol2
        GITHUB_REPOSITORY ThePhD/sol2
        VERSION 3.5.0
)

# sol2 3.3.1 optional<T&>::emplace calls construct(), which that specialization
# does not define. Clang 19+ rejects the header while parsing it.
file(READ "${sol2_SOURCE_DIR}/include/sol/optional_implementation.hpp" _sol2_optional)
string(FIND "${_sol2_optional}" "this->construct(std::forward<Args>(args)...);\n\t\t}" _sol2_optional_bug)
if (NOT _sol2_optional_bug EQUAL -1)
    string(REPLACE
            "\t\ttemplate <class... Args>\n\t\tT& emplace(Args&&... args) noexcept {\n\t\t\tstatic_assert(std::is_constructible<T, Args&&...>::value, \"T must be constructible with Args\");\n\n\t\t\t*this = nullopt;\n\t\t\tthis->construct(std::forward<Args>(args)...);\n\t\t}"
            "\t\tT& emplace(T& arg) noexcept {\n\t\t\tm_value = std::addressof(arg);\n\t\t\treturn *m_value;\n\t\t}"
            _sol2_optional "${_sol2_optional}")
    file(WRITE "${sol2_SOURCE_DIR}/include/sol/optional_implementation.hpp" "${_sol2_optional}")
endif ()

option(EDITOR_BUILD "Build triggered by the Editor for a specific game" OFF)
option(OPENFRONT_BUILD "Build the Openfront sample game" OFF)
if (CMAKE_CROSSCOMPILING)
    set(ASSET_BUNDLER_EXECUTABLE "${CMAKE_BINARY_DIR}/AssetBundler")
else ()
    set(ASSET_BUNDLER_EXECUTABLE "$<TARGET_FILE:AssetBundler>")
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sASYNCIFY")
    set(ASSET_BUNDLER_EXECUTABLE "${CMAKE_BINARY_DIR}/AssetBundler")

    add_compile_options(-fexceptions)
    add_link_options(-fexceptions)

    # Embed game assets and data into the WASM .data file so they are
    # available at runtime in the virtual filesystem.
    set(_wasm_embed_files
            "${CMAKE_SOURCE_DIR}/R-Type/assets@assets"
            "${CMAKE_SOURCE_DIR}/R-Type/scripts@scripts"
            "${CMAKE_SOURCE_DIR}/R-Type/Prefabs@Prefabs"
            "${CMAKE_SOURCE_DIR}/R-Type/project.json@project.json"
            "${CMAKE_SOURCE_DIR}/R-Type/input.json@input.json"
            "${CMAKE_SOURCE_DIR}/UI/lua/templates@UI/lua/templates"
    )
    file(GLOB _wasm_scenes "${CMAKE_SOURCE_DIR}/R-Type/*.scene")
    foreach (_scene ${_wasm_scenes})
        get_filename_component(_scene_name "${_scene}" NAME)
        list(APPEND _wasm_embed_files "${_scene}@${_scene_name}")
    endforeach ()
    foreach (_embed ${_wasm_embed_files})
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --embed-file ${_embed}")
    endforeach ()
else ()
    set(ASSET_BUNDLER_EXECUTABLE "$<TARGET_FILE:AssetBundler>")
endif ()

add_subdirectory(Engine)
add_subdirectory(UI)
add_subdirectory(Serialization)
add_subdirectory(Plugins)
add_subdirectory(Editor)
add_subdirectory(Tools)
add_subdirectory(Games)
add_subdirectory(Tests)
add_subdirectory(Openfront)

if (EDITOR_BUILD)
    if (DEFINED GAME_PROJECT_DIR AND EXISTS "${GAME_PROJECT_DIR}/CMakeLists.txt")
        add_subdirectory("${GAME_PROJECT_DIR}" "${CMAKE_BINARY_DIR}/game_project")
    elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/R-Type/CMakeLists.txt")
        add_subdirectory(R-Type)
    endif ()
endif ()

