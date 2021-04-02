include_directories("${BEEZER_SOURCE_DIR}/TarArchiver")

haiku_add_addon(ark_xz XzArchiver.cpp XzArchiver.rdef)

target_link_libraries(ark_xz ark_tar_static)

set_property(TARGET ark_xz PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_xz-APP_MIME_SIG" "x-vnd.BeezerAddOn-XzArchiver")
	set("ark_xz-LOCALES" "en")
	target_link_libraries(ark_xz "localestub")
	haiku_add_i18n(ark_xz)
endif()
