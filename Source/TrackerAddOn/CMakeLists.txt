set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BEEZER_BUILD_DIR}/TrackerAddOn)
set(HAIKU_CATALOG_BUILD_DIR ${BEEZER_BUILD_DIR}/TrackerAddOn)

haiku_add_executable(BeezerAddOn
	TrackerAddOn.rdef
	TrackerAddOn.cpp
)

target_link_libraries(BeezerAddOn "be")

if(HAIKU_ENABLE_I18N)
	set("BeezerAddOn-APP_MIME_SIG" "x-vnd.Ram-BeezerTA")
	set("BeezerAddOn-LOCALES" "en")
	target_link_libraries(BeezerAddOn "localestub")
	haiku_add_i18n(BeezerAddOn)
endif()
