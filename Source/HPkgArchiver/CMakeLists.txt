haiku_add_addon(ark_hpkg HPkgArchiver.cpp HPkgArchiver.rdef)

#target_link_libraries(ark_hpkg)

set_property(TARGET ark_hpkg PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_hpkg-APP_MIME_SIG" "x-vnd.BeezerAddOn-HPkgArchiver")
	set("ark_hpkg-LOCALES" "en")
	target_link_libraries(ark_hpkg "localestub")
	haiku_add_i18n(ark_hpkg)
endif()
