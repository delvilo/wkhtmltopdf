if(NOT APP OR NOT OUTPUT OR NOT GZIP)
    message(FATAL_ERROR "APP, OUTPUT and GZIP are required")
endif()

execute_process(COMMAND "${APP}" --manpage OUTPUT_FILE "${OUTPUT}.tmp"
    RESULT_VARIABLE manpage_result)
if(NOT manpage_result EQUAL 0)
    file(REMOVE "${OUTPUT}.tmp")
    message(FATAL_ERROR "Could not generate manpage from ${APP}")
endif()

execute_process(COMMAND "${GZIP}" -n -f "${OUTPUT}.tmp"
    RESULT_VARIABLE gzip_result)
if(NOT gzip_result EQUAL 0)
    file(REMOVE "${OUTPUT}.tmp" "${OUTPUT}.tmp.gz")
    message(FATAL_ERROR "Could not compress manpage for ${APP}")
endif()
file(RENAME "${OUTPUT}.tmp.gz" "${OUTPUT}")
