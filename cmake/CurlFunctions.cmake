find_program(CURL_EXECUTABLE curl)
if(CURL_EXECUTABLE STREQUAL "CURL_EXECUTABLE-NOT_FOUND")
    message(FATAL_ERROR "curl is not available")
endif()

# Downloads a url to a file.
# Optionally reads the file to a variable.
# Optionally sends extra headers.
# Will use ETag to avoid downloading again.
# Usage:
#   curl_download(URL url FILE path VAR name)
#   curl_download(URL url FILE path VAR name HEADERS "Authorization: token XXX")
function(curl_download)
    set(options)
    set(oneValueArgs URL FILE VAR)
    set(multiValueArgs HEADERS)
    cmake_parse_arguments(CURL_DOWNLOAD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    if("${CURL_DOWNLOAD_URL}" STREQUAL "" OR "${CURL_DOWNLOAD_FILE}" STREQUAL "")
        message(FATAL_ERROR "the curl_download function requires the URL and FILE arguments")
    endif()

    message("-- Downloading ${CURL_DOWNLOAD_URL}")
    set(http_file "${CURL_DOWNLOAD_FILE}.http")
    set(tmp_file "${CURL_DOWNLOAD_FILE}.tmp")

    set(command ${CURL_EXECUTABLE} "${CURL_DOWNLOAD_URL}" "-L" "-D" ${http_file} "-o" ${tmp_file})
    if(EXISTS ${http_file} AND EXISTS ${CURL_DOWNLOAD_FILE})
        file(STRINGS ${http_file} header LIMIT_COUNT 1 REGEX "ETag:")
        if(NOT "${header}" STREQUAL "")
            message("-- ${header}")
            string(REPLACE "ETag:" "If-None-Match:" header ${header})
            list(APPEND CURL_DOWNLOAD_HEADERS ${header})
        endif()
    endif()
    foreach(header ${CURL_DOWNLOAD_HEADERS})
        list(APPEND command "-H" "${header}")
    endforeach()

    execute_process(COMMAND ${command} RESULT_VARIABLE result)

    if(NOT ${result} EQUAL 0)
        message(FATAL_ERROR "curl failed\nresult: ${result}")
    endif()

    file(STRINGS "${http_file}" header REGEX "HTTP/")
    list(REVERSE header)
    list(GET header 0 header)
    message("-- ${header}")
    if(header MATCHES "HTTP/[^ ]+ 200") # ok
        message("-- Writing ${CURL_DOWNLOAD_FILE}")
        if(EXISTS "${CURL_DOWNLOAD_FILE}")
            file(REMOVE "${CURL_DOWNLOAD_FILE}")
        endif()
        file(RENAME "${tmp_file}" "${CURL_DOWNLOAD_FILE}")
    elseif(header MATCHES "HTTP/[^ ]+ 304") # not modified
        file(REMOVE "${tmp_file}")
    else()
        file(READ "${http_file}" http)
        message(FATAL_ERROR "unexpected reply\nresult: ${result}\nhttp: ${http}")
    endif()

    if(NOT "${CURL_DOWNLOAD_VAR}" STREQUAL "")
        message("-- Reading ${CURL_DOWNLOAD_FILE}")
        file(READ "${CURL_DOWNLOAD_FILE}" output)
        set("${CURL_DOWNLOAD_VAR}" "${output}" PARENT_SCOPE)
    endif()

endfunction(curl_download)
