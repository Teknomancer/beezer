
add_library(ark_rar MODULE RarArchiver.cpp)

#target_link_libraries(ark_rar)

set_property(TARGET ark_rar PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
