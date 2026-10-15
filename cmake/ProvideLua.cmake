# ProvideLua.cmake — make Lua 5.4 available as PkgConfig::LUA on every platform.
#
#   include(ProvideLua)
#   provide_lua()
#
# Both platforms end up running the exact same pkg_check_modules() call:
# on Windows the missing lua5.4.pc is generated at configure time from the
# winget install (DEVCOM.Lua) into <build>/pkgconfig and put on
# PKG_CONFIG_PATH, so the main CMakeLists files contain no Lua ifs at all.
# Also records LUA_PREFIX in the cache for consumers (DLL bundling) and
# copies lua54.dll next to the executables (Windows has no rpath).
function(provide_lua)
    find_package(PkgConfig REQUIRED)
    if(WIN32)
        if(NOT DEFINED CACHE{LUA_PREFIX})
            find_path(_lua_include NAMES lua.h
                PATHS
                "$ENV{LOCALAPPDATA}/Programs/Lua/include"
                "C:/Program Files/Lua/include"
                "C:/Program Files (x86)/Lua/include"
                DOC "Lua 5.4 headers")
            if(NOT _lua_include)
                message(FATAL_ERROR
                    "Lua 5.4 not found. Install it with: winget install --id DEVCOM.Lua")
            endif()
            get_filename_component(_lua_prefix "${_lua_include}" DIRECTORY)
            set(LUA_PREFIX "${_lua_prefix}"
                CACHE PATH "Lua 5.4 install prefix (winget DEVCOM.Lua)")
        endif()
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/pkgconfig")
        configure_file("${CMAKE_SOURCE_DIR}/cmake/pkgconfig/lua5.4.pc.in"
            "${CMAKE_BINARY_DIR}/pkgconfig/lua5.4.pc" @ONLY)
        set(ENV{PKG_CONFIG_PATH} "${CMAKE_BINARY_DIR}/pkgconfig;$ENV{PKG_CONFIG_PATH}")
    endif()
    pkg_check_modules(LUA REQUIRED IMPORTED_TARGET lua5.4)
    if(WIN32)
        # lua54.dll must sit next to the .exe files at runtime (build tree +
        # plain `cmake --build build` runs). Copy it once at configure time.
        foreach(_lua_dll "$CACHE{LUA_PREFIX}/bin/lua54.dll")
            if(EXISTS "${_lua_dll}")
                file(COPY "${_lua_dll}" DESTINATION "${CMAKE_BINARY_DIR}")
                message(STATUS "Bundling ${_lua_dll}")
            endif()
        endforeach()
    endif()
endfunction()
