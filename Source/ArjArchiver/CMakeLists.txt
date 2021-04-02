
haiku_add_addon(ark_arj ArjArchiver.cpp ArjArchiver.rdef)

#target_link_libraries(ark_arj)

set_property(TARGET ark_arj PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_arj-APP_MIME_SIG" "x-vnd.BeezerAddOn-ArjArchiver")
	set("ark_arj-LOCALES" "en")
	target_link_libraries(ark_arj "localestub")
	haiku_add_i18n(ark_arj)
endif()
