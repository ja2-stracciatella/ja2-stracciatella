# \file dependencies/lib-sol2/CMakeLists.txt

message(STATUS "<magic_enum>")

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-magic_enum/src")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")

set(MAGICENUM_INCLUDE_DIR ${SRC_DIR}/include PARENT_SCOPE)

message(STATUS "</magic_enum>")
