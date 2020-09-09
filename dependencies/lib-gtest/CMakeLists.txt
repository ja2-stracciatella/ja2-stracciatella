# \file dependencies/lib-gtest/CMakeLists.txt

message(STATUS "<gtest>")

# create getter
set(SRC_DIR "${CMAKE_BINARY_DIR}/lib-gtest/src")
set(BUILD_DIR "${CMAKE_BINARY_DIR}/lib-gtest/build")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/getter/CMakeLists.txt.in"
    "${CMAKE_CURRENT_BINARY_DIR}/getter/CMakeLists.txt"
    @ONLY
)

# execute getter
execute_process(COMMAND ${CMAKE_COMMAND} . "-G${CMAKE_GENERATOR}" "-DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}" WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/getter")
execute_process(COMMAND ${CMAKE_COMMAND} --build "${CMAKE_CURRENT_BINARY_DIR}/getter")

# library
set(BUILD_GMOCK OFF CACHE INTERNAL "no gmock" FORCE)
set(INSTALL_GTEST OFF CACHE INTERNAL "no install, gtest is integrated" FORCE)
set(gtest_force_shared_crt ON CACHE INTERNAL "always use shared run-time lib (DLL)" FORCE)
set(gtest_disable_pthreads ON CACHE INTERNAL "no pthreads in gtest" FORCE)
set(gtest_build_samples OFF CACHE INTERNAL "default value" FORCE)
set(gtest_build_tests OFF CACHE INTERNAL "default value" FORCE)
set(gtest_hide_internal_symbols OFF CACHE INTERNAL "default value" FORCE)
add_subdirectory("${SRC_DIR}" "${BUILD_DIR}")
set_target_properties(gtest gtest_main PROPERTIES EXCLUDE_FROM_ALL TRUE)
set(GTEST_LIBRARIES gtest PARENT_SCOPE)

message(STATUS "</gtest>")
