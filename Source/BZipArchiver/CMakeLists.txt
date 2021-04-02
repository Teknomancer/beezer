include_directories("${BEEZER_SOURCE_DIR}/TarArchiver")

haiku_add_addon(ark_bzip2 BZipArchiver.cpp BZipArchiver.rdef)

target_link_libraries(ark_bzip2 ark_tar_static)

set_property(TARGET ark_bzip2 PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_bzip2-APP_MIME_SIG" "x-vnd.BeezerAddOn-BZip2Archiver")
	set("ark_bzip2-LOCALES" "en")
	target_link_libraries(ark_bzip2 "localestub")
	haiku_add_i18n(ark_bzip2)
endif()
