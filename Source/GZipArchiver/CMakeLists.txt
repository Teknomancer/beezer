include_directories("${BEEZER_SOURCE_DIR}/TarArchiver")

add_library(ark_gzip MODULE GZipArchiver.cpp)

target_link_libraries(ark_gzip ark_tar)

set_property(TARGET ark_gzip PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
