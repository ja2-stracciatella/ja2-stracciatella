message(STATUS "Entering launcher directory")

option(BUILD_LAUNCHER "Build the ja2 launcher application" ON)
option(LOCAL_FLTK_LIB "Download and build fltk library instead of using system-wide version" OFF)

if(NOT BUILD_LAUNCHER)
    return()
endif()

if (NOT LOCAL_FLTK_LIB)
    find_package(FLTK)
    if(NOT FLTK_FOUND)
        message(STATUS "FLTK not found. Disabling launcher.")
        set(BUILD_LAUNCHER OFF)
    endif()
else()
    set(FLTK_LIBRARIES "")
    set(FLTK_INCLUDE_DIR "")
    add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/lib-fltk" "${CMAKE_BINARY_DIR}/dependencies/lib-fltk" SYSTEM)
endif()
message(STATUS "Fltk Libraries: ${FLTK_LIBRARIES}")

file(GLOB LOCAL_LAUNCHER_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(${LAUNCHER_BINARY}
    "${CMAKE_SOURCE_DIR}/src/sgp/FileMan.cc"
    "${CMAKE_SOURCE_DIR}/src/sgp/SGPFile.cc"
    "${CMAKE_SOURCE_DIR}/src/sgp/SGPStrings.cc"
    ${LOCAL_LAUNCHER_HEADERS}
    StracciatellaLauncher.cc
    Launcher.cc
    main.cc
)

# Ensure the launcher executable is build in the same directory as the game.
set_target_properties(${LAUNCHER_BINARY}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_dependencies(${LAUNCHER_BINARY} stracciatella)

target_link_libraries(${LAUNCHER_BINARY} PRIVATE
    ${FLTK_LIBRARIES}
    ${STRACCIATELLA_LIBRARIES}
    ${LAUNCHER_LIBRARIES}
    ${SDL2_LIBRARY}
)

target_include_directories(${LAUNCHER_BINARY} PRIVATE
    ${FLTK_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    "${CMAKE_SOURCE_DIR}/src/externalized"
    "${CMAKE_SOURCE_DIR}/src/game"
    "${CMAKE_SOURCE_DIR}/src/sgp"
)

add_warnings(${LAUNCHER_BINARY})
