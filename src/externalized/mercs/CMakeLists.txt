file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(LOCAL_JA2_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/BinaryProfileData.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/MercProfile.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/MercProfileInfo.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/MERCListingModel.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/RPCSmallFaceModel.cc
        )

set(JA2_SOURCES
        ${JA2_SOURCES}
        ${LOCAL_JA2_HEADERS}
        ${LOCAL_JA2_SOURCES}
        PARENT_SCOPE
        )
