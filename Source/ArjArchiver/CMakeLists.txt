
add_library(ark_arj MODULE ArjArchiver.cpp)

#target_link_libraries(ark_arj)

set_property(TARGET ark_arj PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
