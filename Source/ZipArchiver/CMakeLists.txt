
haiku_add_addon(ark_zip ZipArchiver.cpp ZipArchiver.rdef)

#target_link_libraries(ark_zip)

set_property(TARGET ark_zip PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_zip-APP_MIME_SIG" "x-vnd.BeezerAddOn-ZipArchiver")
	set("ark_zip-LOCALES" "en")
	target_link_libraries(ark_zip "localestub")
	haiku_add_i18n(ark_zip)
endif()
