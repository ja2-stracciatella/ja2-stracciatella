# find doxygen
find_package(Doxygen REQUIRED)

# set the input and output documentation paths
set(DOXYGEN_INPUT_DIR "${PROJECT_SOURCE_DIR}")
set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/doc")

# configure the source doxyfile
set(DOXYGEN_CONFIGURED_INPUT "${DOXYGEN_OUTPUT_DIR}/doxyfile")
configure_file("${DOXYGEN_INPUT_DIR}/doc/doxyfile.in" "${DOXYGEN_CONFIGURED_INPUT}")

# target
add_custom_target(doc ALL "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_CONFIGURED_INPUT}"
    WORKING_DIRECTORY "${DOXYGEN_INPUT_DIR}"
    COMMENT "Generating API documentation with Doxygen" VERBATIM
)
