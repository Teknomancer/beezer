
haiku_add_addon(ark_squashfs SquashFSArchiver.cpp SquashFSArchiver.rdef)

#target_link_libraries(ark_squashfs)

set_property(TARGET ark_squashfs PROPERTY LIBRARY_OUTPUT_DIRECTORY ${BEEZER_BUILD_ADDONS_DIR})

if(HAIKU_ENABLE_I18N)
	set("ark_squashfs-APP_MIME_SIG" "x-vnd.BeezerAddOn-SquashFSArchiver")
	set("ark_squashfs-LOCALES" "en")
	target_link_libraries(ark_squashfs "localestub")
	haiku_add_i18n(ark_squashfs)
endif()
