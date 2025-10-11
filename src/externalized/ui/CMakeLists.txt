file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(LOCAL_JA2_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/Easings.cc
        )

if (WITH_UNITTESTS)
    set(LOCAL_JA2_SOURCES
            ${LOCAL_JA2_SOURCES}
            ${CMAKE_CURRENT_SOURCE_DIR}/Easings_unittests.cc
            )
endif()

set(JA2_SOURCES
        ${JA2_SOURCES}
        ${LOCAL_JA2_HEADERS}
        ${LOCAL_JA2_SOURCES}
        PARENT_SCOPE
        )
