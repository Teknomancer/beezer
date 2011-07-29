
add_library(ark_tar MODULE TarArchiver.cpp)

#target_link_libraries(ark_tar)

set_property(TARGET ark_tar PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

add_library(ark_tar_static TarArchiver.cpp)

set_property(TARGET ark_tar_static PROPERTY COMPILE_DEFINITIONS STATIC_LIB_BUILD)
