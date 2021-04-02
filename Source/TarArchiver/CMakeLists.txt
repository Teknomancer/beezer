
haiku_add_addon(ark_tar TarArchiver.cpp TarArchiver.rdef)

#target_link_libraries(ark_tar)

set_property(TARGET ark_tar PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

add_library(ark_tar_static TarArchiver.cpp)

set_property(TARGET ark_tar_static PROPERTY COMPILE_DEFINITIONS STATIC_LIB_BUILD)

if(HAIKU_ENABLE_I18N)
	set("ark_tar-APP_MIME_SIG" "x-vnd.BeezerAddOn-TarArchiver")
	set("ark_tar-LOCALES" "en")
	target_link_libraries(ark_tar "localestub")
	haiku_add_i18n(ark_tar)
endif()
