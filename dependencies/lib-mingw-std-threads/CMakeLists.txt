# \file dependencies/lib-mingw-std-threads/CMakeLists.txt

message(STATUS "<mingw-std-threads>")

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-mingw-std-threads/src")
set(BUILD_DIR "${CMAKE_BINARY_DIR}/lib-mingw-std-threads/build")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")

option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
add_subdirectory(${SRC_DIR} ${BUILD_DIR})

message(STATUS "</mingw-std-threads>")
