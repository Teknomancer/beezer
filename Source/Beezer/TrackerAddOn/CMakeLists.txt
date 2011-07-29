set(EXECUTABLE_OUTPUT_PATH ${BEEZER_BUILD_DIR}/TrackerAddOn)

add_executable(BeezerAddOn
	TrackerAddOn.rdef
	TrackerAddOn.cpp
)

target_link_libraries(BeezerAddOn "be")
