
haiku_add_addon(ark_rar RarArchiver.cpp RarArchiver.rdef)

#target_link_libraries(ark_rar)

set_property(TARGET ark_rar PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_rar-APP_MIME_SIG" "x-vnd.BeezerAddOn-RarArchiver")
	set("ark_rar-LOCALES" "en")
	target_link_libraries(ark_rar "localestub")
	haiku_add_i18n(ark_rar)
endif()
