include_directories("${BEEZER_SOURCE_DIR}/TarArchiver")

haiku_add_addon(ark_gzip GZipArchiver.cpp GZipArchiver.rdef)

target_link_libraries(ark_gzip ark_tar_static)

set_property(TARGET ark_gzip PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_gzip-APP_MIME_SIG" "x-vnd.BeezerAddOn-GZipArchiver")
	set("ark_gzip-LOCALES" "en")
	target_link_libraries(ark_gzip "localestub")
	haiku_add_i18n(ark_gzip)
endif()
