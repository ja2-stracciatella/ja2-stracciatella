# files
file(GLOB_RECURSE header_files "${PROJECT_SOURCE_DIR}/include/MicroIni/*")
file(GLOB_RECURSE source_files "${PROJECT_SOURCE_DIR}/src/MicroIni/*")

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# library
add_library(MicroIni ${header_files} ${source_files})
set_target_properties(MicroIni PROPERTIES DEFINE_SYMBOL MICROINI_EXPORTS)
if(BUILD_SHARED_LIBS)
    set_target_properties(MicroIni PROPERTIES DEBUG_POSTFIX -d)
    if(WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
        set_target_properties(MicroIni PROPERTIES PREFIX "")
        set_target_properties(MicroIni PROPERTIES IMPORT_SUFFIX ".a")
    endif()
else()
    set_target_properties(MicroIni PROPERTIES DEBUG_POSTFIX "-s-d")
    set_target_properties(MicroIni PROPERTIES RELEASE_POSTFIX "-s")
endif()

# install rules
install(FILES ${header_files} DESTINATION include/MicroIni)
install(TARGETS MicroIni
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
