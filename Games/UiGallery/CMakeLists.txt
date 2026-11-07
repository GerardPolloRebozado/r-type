add_executable(UiGallery main.cpp)
target_link_libraries(UiGallery PRIVATE Engine UI)
target_compile_definitions(UiGallery PRIVATE "UI_GALLERY_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
