file(GLOB_RECURSE ASSETS_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        COMMAND AssetBundler ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
        DEPENDS AssetBundler ${ASSETS_LIST}
        COMMENT "Bundling assets for Example..."
)


add_executable(Sounds main.cpp ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp)
target_include_directories(Sounds PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Sounds PRIVATE Engine)
