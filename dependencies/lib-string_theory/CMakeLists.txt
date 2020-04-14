# string_theory library
#
# built as a release static library with an extrenal project at configure time
# installed to "${CMAKE_BINARY_DIR}/lib-string_theory"
# the builder can be replaced by FetchContent in cmake 3.11

message(STATUS "<string_theory>")

# local config
set(CMAKE_BUILD_TYPE Release)
set(CACHE_FILE "${CMAKE_CURRENT_BINARY_DIR}/external-project-cache.cmake")
set(INSTALL_PREFIX "${CMAKE_BINARY_DIR}/lib-string_theory")

configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/external-project-cache.cmake.in"
	"${CACHE_FILE}"
	@ONLY
)

# configure builder
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/builder/CMakeLists.txt.in"
	"${CMAKE_CURRENT_BINARY_DIR}/builder/CMakeLists.txt"
	@ONLY
)
execute_process(
	COMMAND "${CMAKE_COMMAND}" "-C${CACHE_FILE}" .
	RESULT_VARIABLE result
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/builder"
)
if(NOT "${result}" EQUAL 0)
	message(FATAL_ERROR "${result}")
endif()

# run builder
execute_process(
	COMMAND "${CMAKE_COMMAND}" --build . --target string_theory
	RESULT_VARIABLE result
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/builder"
)
if(NOT "${result}" EQUAL 0)
	message(FATAL_ERROR "${result}")
endif()

# interface library
add_library(string_theory INTERFACE)
target_include_directories(string_theory INTERFACE "${INSTALL_PREFIX}/include")

message(STATUS "</string_theory>")
