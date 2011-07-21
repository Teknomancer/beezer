
add_library(ark_tar MODULE TarArchiver.cpp)

#target_link_libraries(ark_tar)

set_property(TARGET ark_tar PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
