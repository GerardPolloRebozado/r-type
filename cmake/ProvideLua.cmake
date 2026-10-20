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
        if(NOT DEFINED CACHE{LUA_PREFIX} OR NOT EXISTS "${LUA_PREFIX}/include/lua.h")
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
                CACHE PATH "Lua 5.4 install prefix (winget DEVCOM.Lua)" FORCE)
        endif()
        # pkgconf relocates the `prefix` variable on Windows from the .pc file
        # location, so a .pc under <build>/pkgconfig makes it point at the repo
        # root. Absolute dirs keep -I/-L independent of that relocation.
        set(LUA_LIBDIR "${LUA_PREFIX}/lib")
        set(LUA_INCLUDEDIR "${LUA_PREFIX}/include")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/pkgconfig")
        configure_file("${CMAKE_SOURCE_DIR}/cmake/pkgconfig/lua5.4.pc.in"
            "${CMAKE_BINARY_DIR}/pkgconfig/lua5.4.pc" @ONLY)
        set(ENV{PKG_CONFIG_PATH} "${CMAKE_BINARY_DIR}/pkgconfig;$ENV{PKG_CONFIG_PATH}")
    endif()
    pkg_check_modules(LUA REQUIRED IMPORTED_TARGET lua5.4)
    # Safety net: never export an include dir that does not exist.
    set(_lua_include_dirs "")
    foreach(_lua_dir IN LISTS LUA_INCLUDE_DIRS)
        if(IS_DIRECTORY "${_lua_dir}")
            list(APPEND _lua_include_dirs "${_lua_dir}")
        endif()
    endforeach()
    if(NOT _lua_include_dirs)
        foreach(_lua_candidate "${LUA_PREFIX}/include" "${LUA_PREFIX}/include/lua5.4")
            if(EXISTS "${_lua_candidate}/lua.h")
                list(APPEND _lua_include_dirs "${_lua_candidate}")
            endif()
        endforeach()
    endif()
    if(_lua_include_dirs)
        set(LUA_INCLUDE_DIRS "${_lua_include_dirs}" PARENT_SCOPE)
        set_target_properties(PkgConfig::LUA PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_lua_include_dirs}")
    endif()
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
