# ProvideLua.cmake — make Lua 5.4 available as PkgConfig::LUA on every platform.
#
#   include(ProvideLua)
#   provide_lua()
#
# On Unix the pkg-config module is used directly. On Windows the target is
# built from LUA_PREFIX (winget DEVCOM.Lua locally, a staged MSYS2 prefix in
# CI): pkgconf enables `--define-prefix` by default on Windows and rewrites
# every path derived from `prefix` to the parent of the .pc file. With the
# generated .pc under <build>/pkgconfig that turned the real
# <prefix>/include into <repo>/include, which CMake then rejected as a
# non-existent include directory. Building the imported target ourselves
# keeps the paths absolute and independent of pkgconf's relocation.
function(provide_lua)
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
        file(TO_CMAKE_PATH "${LUA_PREFIX}" _lua_prefix)
        set(LUA_PREFIX "${_lua_prefix}"
            CACHE PATH "Lua 5.4 install prefix" FORCE)

        if(NOT EXISTS "${LUA_PREFIX}/include/lua.h")
            message(FATAL_ERROR
                "LUA_PREFIX points to '${LUA_PREFIX}', which has no include/lua.h")
        endif()

        find_library(_lua_library NAMES lua54 lua5.4 lua
            PATHS "${LUA_PREFIX}/lib"
            NO_DEFAULT_PATH)
        if(NOT _lua_library)
            message(FATAL_ERROR
                "Lua 5.4 library not found in '${LUA_PREFIX}/lib'")
        endif()

        if(NOT TARGET PkgConfig::LUA)
            add_library(PkgConfig::LUA INTERFACE IMPORTED)
        endif()
        set_target_properties(PkgConfig::LUA PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${LUA_PREFIX}/include"
            INTERFACE_LINK_LIBRARIES "${_lua_library}")

        set(LUA_INCLUDE_DIRS "${LUA_PREFIX}/include" PARENT_SCOPE)

        # lua54.dll must sit next to the .exe files at runtime (build tree +
        # plain `cmake --build build` runs). Copy it once at configure time.
        if(EXISTS "${LUA_PREFIX}/bin/lua54.dll")
            file(COPY "${LUA_PREFIX}/bin/lua54.dll"
                DESTINATION "${CMAKE_BINARY_DIR}")
            message(STATUS "Bundling ${LUA_PREFIX}/bin/lua54.dll")
        endif()
        return()
    endif()

    find_package(PkgConfig REQUIRED)
    if(APPLE)
        # Homebrew's `lua` formula is Lua 5.5 now; 5.4 is the keg-only
        # `lua@5.4`, whose lua5.4.pc is not on pkg-config's default path.
        # Look in its prefix so no -DCMAKE_PREFIX_PATH is needed (editor game
        # builds don't pass one). pkg-config searches CMAKE_PREFIX_PATH.
        set(_brew_lua_prefixes "/opt/homebrew/opt/lua@5.4" "/usr/local/opt/lua@5.4")
        find_program(_brew_executable brew)
        if(_brew_executable)
            execute_process(COMMAND "${_brew_executable}" --prefix lua@5.4
                OUTPUT_VARIABLE _brew_lua_prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
            if(_brew_lua_prefix)
                list(PREPEND _brew_lua_prefixes "${_brew_lua_prefix}")
            endif()
        endif()
        foreach(_brew_lua IN LISTS _brew_lua_prefixes)
            if(EXISTS "${_brew_lua}/lib/pkgconfig/lua5.4.pc")
                list(APPEND CMAKE_PREFIX_PATH "${_brew_lua}")
                break()
            endif()
        endforeach()
    endif()
    pkg_check_modules(LUA IMPORTED_TARGET lua5.4)
    if(NOT LUA_FOUND)
        if(APPLE)
            message(FATAL_ERROR
                "Lua 5.4 not found. Install it with: brew install lua@5.4")
        else()
            message(FATAL_ERROR
                "Lua 5.4 not found (pkg-config module lua5.4). Install it, e.g. "
                "sudo apt install liblua5.4-dev")
        endif()
    endif()
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
endfunction()
