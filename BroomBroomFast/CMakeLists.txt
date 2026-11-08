add_executable(BroomBroomFast main.cpp)
target_link_libraries(BroomBroomFast PRIVATE Engine Serialization UI)
# The build root already has a "BroomBroomFast" folder (this directory's
# build tree), so the executable can't go there.
set_target_properties(BroomBroomFast PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# Scenes, scripts and assets are loaded relative to the executable,
# so copy it next to them.
add_custom_command(TARGET BroomBroomFast POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:BroomBroomFast> "${CMAKE_CURRENT_SOURCE_DIR}/BroomBroomFast"
)
