
add_library(ark_7zip MODULE z7Archiver.cpp)

#target_link_libraries(ark_7zip)

set_property(TARGET ark_7zip PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
