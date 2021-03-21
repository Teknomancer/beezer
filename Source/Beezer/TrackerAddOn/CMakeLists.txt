set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BEEZER_BUILD_DIR}/TrackerAddOn)
set(HAIKU_CATALOG_BUILD_DIR ${BEEZER_BUILD_DIR}/TrackerAddOn)

if(HAIKU_ENABLE_I18N STREQUAL "ON")
	set("BeezerAddOn-APP_MIME_SIG" "x-vnd.Ram-BeezerTA")
	set("BeezerAddOn-LOCALES" "en")
endif()

add_executable(BeezerAddOn
	TrackerAddOn.rdef
	TrackerAddOn.cpp
)

target_link_libraries(BeezerAddOn "be")

if(HAIKU_ENABLE_I18N STREQUAL "ON")
	target_link_libraries(BeezerAddOn "localestub")
endif()
