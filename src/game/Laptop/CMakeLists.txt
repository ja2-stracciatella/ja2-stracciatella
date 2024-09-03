file(GLOB LOCAL_JA2_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(JA2_SOURCES
    ${JA2_SOURCES}
    ${LOCAL_JA2_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/AIM.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/AIMArchives.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/AIMFacialIndex.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/AIMHistory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/AIMLinks.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/AIMMembers.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/AIMPolicies.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/AIMSort.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyR.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyRAmmo.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyRArmour.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyRGuns.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyRMailOrder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyRMisc.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyRShipments.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BobbyRUsed.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/BrokenLink.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/CharProfile.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/EMail.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Files.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Finances.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Florist.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Florist_Cards.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Florist_Gallery.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Florist_Order_Form.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Funeral.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/History.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMPVideoObjects.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_AboutUs.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Attribute_Entrance.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Attribute_Finish.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Attribute_Selection.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Begin_Screen.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Compile_Character.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Confirm.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Finish.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_HomePage.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_MainPage.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Personality_Entrance.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Personality_Finish.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Personality_Quiz.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Portraits.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_SkillTraits.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Text_System.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/IMP_Voices.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Insurance.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Insurance_Comments.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Insurance_Contract.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Insurance_Info.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Laptop.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Mercs.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Mercs_Account.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Mercs_Files.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Mercs_No_Account.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Personnel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/Store_Inventory.cc
    PARENT_SCOPE
)

# Laptop has many files that cannot be compiled together because they all have
# their own MakeButton function, for example. The following group is a
# hand-picked list of unproblematic sources.
set_property(
    SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/AIM.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/AIMArchives.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/AIMFacialIndex.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/AIMLinks.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/AIMSort.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/BobbyR.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/BrokenLink.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/CharProfile.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Funeral.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/IMPVideoObjects.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Laptop.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Mercs.cc
        ${CMAKE_CURRENT_SOURCE_DIR}/Store_Inventory.cc
    DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTY UNITY_GROUP "Laptop"
)

set(JA2_INCLUDES
    ${JA2_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    PARENT_SCOPE
)
