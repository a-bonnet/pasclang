enable_testing()

add_custom_target(runtests
    COMMAND ${CMAKE_COMMAND}
    -DBINARY_DIR=${CMAKE_BINARY_DIR}
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/RunTests.cmake)

add_custom_target(runmoretests
    COMMAND ${CMAKE_COMMAND}
    -DBINARY_DIR=${CMAKE_BINARY_DIR}
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/more
    -P ${CMAKE_CURRENT_SOURCE_DIR}/RunTests.cmake)

