
haiku_add_addon(ark_7zip z7Archiver.cpp z7Archiver.rdef)

#target_link_libraries(ark_7zip)

set_property(TARGET ark_7zip PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_7zip-APP_MIME_SIG" "x-vnd.BeezerAddOn-7ZipArchiver")
	set("ark_7zip-LOCALES" "en")
	target_link_libraries(ark_7zip "localestub")
	haiku_add_i18n(ark_7zip)
endif()
