# \file dependencies/lib-miniaudio/CMakeLists.txt

message(STATUS "<miniaudio>")

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-miniaudio/src")
set(BUILD_DIR "${CMAKE_BINARY_DIR}/lib-miniaudio/build")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")

set(MINIAUDIO_INCLUDE_DIRS ${SRC_DIR} PARENT_SCOPE)

message(STATUS "</miniaudio>")
