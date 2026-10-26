file(GLOB_RECURSE ASSETS_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
    COMMAND ${ASSET_BUNDLER_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp
    DEPENDS AssetBundler ${ASSETS_LIST}
    COMMENT "Bundling assets for R-Type..."
)

add_executable(Game main.cpp ${CMAKE_CURRENT_BINARY_DIR}/BundledAssets.hpp)
target_include_directories(Game PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(Game PRIVATE Engine Serialization)
