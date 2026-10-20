# BundleMingwRuntime.cmake — make MinGW-built .exe files runnable without
# CLion on PATH.
#
#   include(BundleMingwRuntime)
#   bundle_mingw_runtime()
#
# Statically links libgcc/libstdc++ and copies the remaining runtime DLLs
# (libwinpthread-1.dll, plus leftovers) next to the executables, so build-time
# tools (AssetBundler runs during the build) and games start with a plain
# `cmake --build build`. Without this every .exe fails with 0xc0000135.
# NOTE: full `-static` breaks this toolchain (undefined
# __intrinsic_setjmpex / __ms_vsnprintf), so winpthread stays a DLL.
# No-op on non-MinGW toolchains.
function(bundle_mingw_runtime)
    if(NOT MINGW)
        return()
    endif()
    add_link_options(-static-libgcc -static-libstdc++)
    # The MinGW runtime DLLs live next to the compiler driver.
    get_filename_component(_mingw_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach(_dll libwinpthread-1.dll libstdc++-6.dll libgcc_s_seh-1.dll)
        if(EXISTS "${_mingw_bin}/${_dll}")
            file(COPY "${_mingw_bin}/${_dll}" DESTINATION "${CMAKE_BINARY_DIR}")
            message(STATUS "Bundling MinGW runtime ${_mingw_bin}/${_dll}")
        endif()
    endforeach()
endfunction()
