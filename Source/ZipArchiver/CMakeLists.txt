
add_library(ark_zip MODULE ZipArchiver.cpp)

#target_link_libraries(ark_zip)

set_property(TARGET ark_zip PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
