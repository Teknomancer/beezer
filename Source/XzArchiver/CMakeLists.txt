include_directories("${BEEZER_SOURCE_DIR}/TarArchiver")

add_library(ark_xz MODULE XzArchiver.cpp)

target_link_libraries(ark_xz ark_tar_static)

set_property(TARGET ark_xz PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})
