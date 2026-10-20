cmake_minimum_required(VERSION 3.28)
project(AssetBundler)

if (CMAKE_CROSSCOMPILING)
    find_program(ASSETBUNDLER_HOST_CXX NAMES c++ g++ clang++)
    if (NOT ASSETBUNDLER_HOST_CXX)
        message(FATAL_ERROR
                "AssetBundler must be built natively for the host but no host "
                "C++ compiler (c++/g++/clang++) was found.")
    endif ()
    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/AssetBundler
            COMMAND ${ASSETBUNDLER_HOST_CXX} -std=c++23
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            -o ${CMAKE_BINARY_DIR}/AssetBundler
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            COMMENT "Building AssetBundler natively (host tool)"
    )
    add_custom_target(AssetBundler ALL
            DEPENDS ${CMAKE_BINARY_DIR}/AssetBundler)
else ()
    add_executable(AssetBundler main.cpp)
    set_target_properties(AssetBundler PROPERTIES CXX_STANDARD 23)
endif ()
