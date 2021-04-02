include_directories("${BEEZER_SOURCE_DIR}/TarArchiver")

haiku_add_addon(ark_zstd ZstdArchiver.cpp ZstdArchiver.rdef)

target_link_libraries(ark_zstd ark_tar_static)

set_property(TARGET ark_zstd PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_zstd-APP_MIME_SIG" "x-vnd.BeezerAddOn-ZstdArchiver")
	set("ark_zstd-LOCALES" "en")
	target_link_libraries(ark_zstd "localestub")
	haiku_add_i18n(ark_zstd)
endif()
