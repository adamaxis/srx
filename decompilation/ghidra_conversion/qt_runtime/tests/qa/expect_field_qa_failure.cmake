foreach(required_var TEST_EXECUTABLE ASSET_ROOT FIELD_QA_FLAG OUTPUT_DIR
        BUG_ID EXPECTED_FAILURE_PATTERN)
    if(NOT DEFINED ${required_var})
        message(FATAL_ERROR "missing -D${required_var}")
    endif()
endforeach()

execute_process(
    COMMAND "${TEST_EXECUTABLE}" --assets "${ASSET_ROOT}"
        "${FIELD_QA_FLAG}" "${OUTPUT_DIR}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
    TIMEOUT 300)
set(output "${stdout}\n${stderr}")
if(result EQUAL 0)
    message(FATAL_ERROR
        "${BUG_ID}: expected live field QA failure, but the beat passed\n${output}")
endif()
if(NOT output MATCHES "${EXPECTED_FAILURE_PATTERN}")
    message(FATAL_ERROR
        "${BUG_ID}: failure did not match '${EXPECTED_FAILURE_PATTERN}' "
        "(result=${result})\n${output}")
endif()
message(STATUS "${BUG_ID}: observed expected live field QA failure")
