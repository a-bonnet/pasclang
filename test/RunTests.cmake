file(REMOVE_RECURSE "${BINARY_DIR}/check")
file(MAKE_DIRECTORY "${BINARY_DIR}/check")

message(STATUS "Tests consist of comparing generated output of test cases to reference output given reference input.")
message("")
file(GLOB TEST_SOURCES "${SOURCE_DIR}/*.pp")

# test with no optimization
foreach(SOURCE_FILE ${TEST_SOURCES})
    get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME)
    message(STATUS "Testing file ${SOURCE_FILE} without optimization")
    execute_process(COMMAND ${BINARY_DIR}/bin/pasclang ${SOURCE_FILE} -o ${BINARY_DIR}/check/${SOURCE_NAME}.bin)
    execute_process(COMMAND ${BINARY_DIR}/check/${SOURCE_NAME}.bin INPUT_FILE ${SOURCE_FILE}.in OUTPUT_FILE ${BINARY_DIR}/check/${SOURCE_NAME}.out)
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${SOURCE_FILE}.out ${BINARY_DIR}/check/${SOURCE_NAME}.out RESULT_VARIABLE TEST_RESULT)
    if(TEST_RESULT)
        message(SEND_ERROR "Failed!")
    elseif(NOT TEST_RESULT)
        message(STATUS "Passed!")
    endif(TEST_RESULT)
    message("")
endforeach(SOURCE_FILE ${TEST_SOURCES})

# test with optimization on
foreach(SOURCE_FILE ${TEST_SOURCES})
    get_filename_component(SOURCE_NAME ${SOURCE_FILE} NAME)
    message(STATUS "Testing file ${SOURCE_FILE} with optimization")
    execute_process(COMMAND ${BINARY_DIR}/bin/pasclang -O1 ${SOURCE_FILE} -o ${BINARY_DIR}/check/${SOURCE_NAME}.opt.bin)
    execute_process(COMMAND ${BINARY_DIR}/check/${SOURCE_NAME}.opt.bin INPUT_FILE ${SOURCE_FILE}.in OUTPUT_FILE ${BINARY_DIR}/check/${SOURCE_NAME}.opt.out)
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files ${SOURCE_FILE}.out ${BINARY_DIR}/check/${SOURCE_NAME}.opt.out RESULT_VARIABLE TEST_RESULT)
    if(TEST_RESULT)
        message(SEND_ERROR "Failed!")
    elseif(NOT TEST_RESULT)
        message(STATUS "Passed!")
    endif(TEST_RESULT)
    message("")
endforeach(SOURCE_FILE ${TEST_SOURCES})

message(STATUS "All tests passed if CMake reports no error.")

