# BakeEditorToolchain.cmake — let exported games reuse the editor's toolchain.
#
#   include(BakeEditorToolchain)
#   bake_editor_toolchain(RTypeEditor)
#
# The editor generates out-of-tree game builds (project/.build). Without
# explicit flags Windows silently falls back to Visual Studio, which was never
# validated for this codebase. Baking the editor's own generator/compiler
# paths as compile definitions lets SettingsPanel pass them through, and the
# build thread also gets the runtime DLL locations for the game's folder.
function(bake_editor_toolchain target)
    file(TO_CMAKE_PATH "${CMAKE_C_COMPILER}" _baked_c)
    file(TO_CMAKE_PATH "${CMAKE_CXX_COMPILER}" _baked_cxx)
    file(TO_CMAKE_PATH "${CMAKE_MAKE_PROGRAM}" _baked_make)
    target_compile_definitions(${target} PRIVATE
        EDITOR_CMAKE_GENERATOR="${CMAKE_GENERATOR}"
        EDITOR_CMAKE_C_COMPILER="${_baked_c}"
        EDITOR_CMAKE_CXX_COMPILER="${_baked_cxx}"
        EDITOR_CMAKE_MAKE_PROGRAM="${_baked_make}"
    )
    if(MINGW)
        get_filename_component(_mingw_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
        file(TO_CMAKE_PATH "${_mingw_bin}" _mingw_bin)
    else()
        set(_mingw_bin "")
    endif()
    target_compile_definitions(${target} PRIVATE
        EDITOR_MINGW_BIN_DIR="${_mingw_bin}"
    )
    if(WIN32)
        # Reuses the prefix found by provide_lua (Engine runs first).
        set(_lua_dll "")
        if(DEFINED CACHE{LUA_PREFIX} AND EXISTS "$CACHE{LUA_PREFIX}/bin/lua54.dll")
            file(TO_CMAKE_PATH "$CACHE{LUA_PREFIX}/bin/lua54.dll" _lua_dll)
        endif()
        target_compile_definitions(${target} PRIVATE
            EDITOR_LUA_DLL="${_lua_dll}"
        )
    endif()
endfunction()
