file(GLOB_RECURSE ASSETS_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
    COMMAND AssetBundler ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
    DEPENDS AssetBundler ${ASSETS_LIST}
    COMMENT "Bundling assets for Text..."
)

add_executable(Text main.cpp ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp)
target_include_directories(Text PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Text PRIVATE Engine)
