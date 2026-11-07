add_executable(R-Type "main.cpp" ${RTYPE_PROJECT_SYSTEM_SOURCES})
target_link_libraries(R-Type PRIVATE Engine Serialization UI)
add_custom_command(TARGET R-Type POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:R-Type> "/home/gerard/epitech/Tek3/B-CPP-500/r-type/R-Type/R-Type"
)
