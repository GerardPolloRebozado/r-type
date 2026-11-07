add_executable(R-Type "main.cpp" ${RTYPE_PROJECT_SYSTEM_SOURCES})
target_link_libraries(R-Type PRIVATE Engine Serialization UI)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # Put the executable in a dedicated bin dir so macOS doesn't conflict with
    # the R-Type/build/R-Type/ directory CMake creates for target internals.
    set_target_properties(R-Type PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    add_custom_command(TARGET R-Type POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:R-Type> "${CMAKE_CURRENT_SOURCE_DIR}/R-Type"
    )
endif()
