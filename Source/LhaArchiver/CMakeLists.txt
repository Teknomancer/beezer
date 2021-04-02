
haiku_add_addon(ark_lha LhaArchiver.cpp LhaArchiver.rdef)

#target_link_libraries(ark_lha)

set_property(TARGET ark_lha PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_lha-APP_MIME_SIG" "x-vnd.BeezerAddOn-LhaArchiver")
	set("ark_lha-LOCALES" "en")
	target_link_libraries(ark_lha "localestub")
	haiku_add_i18n(ark_lha)
endif()
