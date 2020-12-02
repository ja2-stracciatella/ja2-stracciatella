# \file dependencies/lib-lua/CMakeLists.txt
message(STATUS "<lua>")

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(lua)

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-lua/src")
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
        "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
        @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")


# include Lua source files in project
if(UNIX AND NOT MINGW)
    add_definitions("-DLUA_USE_POSIX")
endif()

set(LUA_INCLUDE_DIRS ${SRC_DIR}/src PARENT_SCOPE)

FILE(GLOB LUA_SRC "${SRC_DIR}/src/*.c")
LIST(REMOVE_ITEM LUA_SRC
    "${SRC_DIR}/src/lua.c"
    "${SRC_DIR}/src/luac.c")

ADD_LIBRARY(lua ${LUA_SRC})
message(STATUS "</lua>")