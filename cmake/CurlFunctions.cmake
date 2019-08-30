find_program(CURL_EXECUTABLE "curl${CMAKE_EXECUTABLE_SUFFIX}")
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

    set(command ${CURL_EXECUTABLE} "${CURL_DOWNLOAD_URL}" "-sSL" "-D" ${http_file})
    if(EXISTS ${http_file} AND EXISTS ${CURL_DOWNLOAD_FILE})
        file(STRINGS ${http_file} header LIMIT_COUNT 1 REGEX "ETag:")
        if(NOT "${header}" STREQUAL "")
            message("-- ${header}")
            string(REPLACE "ETag:" "If-None-Match:" header ${header})
            list(APPEND CURL_DOWNLOAD_HEADERS ${header})
        endif()
    endif()
    foreach(header ${CURL_DOWNLOAD_HEADERS})
        set(command ${command} "-H" "${header}")
    endforeach()

    execute_process(COMMAND ${command} RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)

    if(NOT ${result} EQUAL 0)
        message(FATAL_ERROR "curl failed\nresult: ${result}\noutput: ${output}\nerror: ${error}")
    endif()

    file(STRINGS ${http_file} header REGEX "HTTP/")
    list(REVERSE header)
    list(GET header 0 header)
    message("-- ${header}")
    if(header MATCHES "HTTP/[^ ]+ 304")
        message("-- Reading ${CURL_DOWNLOAD_FILE}")
        file(READ ${CURL_DOWNLOAD_FILE} output)
    elseif(header MATCHES "HTTP/[^ ]+ 200")
        message("-- Writing ${CURL_DOWNLOAD_FILE}")
        file(WRITE ${CURL_DOWNLOAD_FILE} "${output}")
    else()
        file(READ ${http_file} http)
        message(FATAL_ERROR "unexpected reply\nresult: ${result}\noutput: ${output}\nerror: ${error}\nhttp: ${http}")
    endif()

    if(NOT "${CURL_DOWNLOAD_VAR}" STREQUAL "")
        set("${CURL_DOWNLOAD_VAR}" "${output}" PARENT_SCOPE)
    endif()

endfunction(curl_download)
