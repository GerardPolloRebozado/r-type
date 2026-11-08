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
if (NOT CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm_cache" CACHE PATH "CPM source cache")
endif ()
# GitHub release downloads sometimes return an empty body (HTTP 504) without
# failing file(DOWNLOAD). Including that empty script leaves CPMAddPackage
# undefined. Fall back to the CPM source cache, then to the raw module.
set(_cpm_bootstrap "${CMAKE_CURRENT_BINARY_DIR}/cpm/get_cpm.cmake")
file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/get_cpm.cmake
        ${_cpm_bootstrap}
        STATUS _cpm_dl
        TIMEOUT 20
)
set(_cpm_bytes 0)
if (EXISTS "${_cpm_bootstrap}")
    file(SIZE "${_cpm_bootstrap}" _cpm_bytes)
endif ()
list(GET _cpm_dl 0 _cpm_dl_code)
if (NOT _cpm_dl_code EQUAL 0 OR _cpm_bytes LESS 100)
    set(_cpm_cached "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
    set(_cpm_module "${CMAKE_CURRENT_BINARY_DIR}/cpm/CPM.cmake")
    if (EXISTS "${_cpm_cached}")
        set(_cpm_module "${_cpm_cached}")
    else ()
        file(DOWNLOAD
                https://raw.githubusercontent.com/cpm-cmake/CPM.cmake/v${CPM_DOWNLOAD_VERSION}/cmake/CPM.cmake
                ${_cpm_module}
                STATUS _cpm_dl
                TIMEOUT 30
        )
        list(GET _cpm_dl 0 _cpm_dl_code)
        if (NOT _cpm_dl_code EQUAL 0)
            message(FATAL_ERROR "Failed to download CPM.cmake: ${_cpm_dl}")
        endif ()
    endif ()
    include(${_cpm_module})
else ()
    include(${_cpm_bootstrap})
endif ()

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
else ()
    set(ASSET_BUNDLER_EXECUTABLE "$<TARGET_FILE:AssetBundler>")
endif ()

add_subdirectory(Engine)
add_subdirectory(UI)
add_subdirectory(Serialization)
add_subdirectory(Plugins)

if (NOT EDITOR_BUILD)
    # Tools before Editor so RTypeEditor can link the Author library.
    add_subdirectory(Tools)
    add_subdirectory(Editor)
    add_subdirectory(Games)
    add_subdirectory(Tests)
endif ()

if (EDITOR_BUILD)
    include(.editor_build.cmake OPTIONAL)
endif ()
