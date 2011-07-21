include_directories("${BEEZER_SOURCE_DIR}/TarArchiver")

add_library(ark_bzip2 MODULE BZipArchiver.cpp)

target_link_libraries(ark_bzip2 ark_tar)

set_property(TARGET ark_bzip2 PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
