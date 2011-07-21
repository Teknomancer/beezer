
add_library(ark_lha MODULE LhaArchiver.cpp)

#target_link_libraries(ark_lha)

set_property(TARGET ark_lha PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
