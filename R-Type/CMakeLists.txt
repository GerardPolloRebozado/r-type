add_executable(Game "main.cpp" ${RTYPE_PROJECT_SYSTEM_SOURCES})
target_link_libraries(Game PRIVATE Engine Serialization UI)
add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Game> "/home/gerard/epitech/Tek3/B-CPP-500/r-type/R-Type/Game"
)
