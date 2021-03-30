set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BEEZER_BUILD_DIR}/FileJoinerStub)
set(HAIKU_CATALOG_BUILD_DIR ${BEEZER_BUILD_DIR}/FileJoinerStub)

if(HAIKU_ENABLE_I18N STREQUAL "ON")
	set("FileJoinerStub-APP_MIME_SIG" "x-vnd.Ram-FileJoiner")
	set("FileJoinerStub-LOCALES" "en")
endif()

add_executable(FileJoinerStub
	FileJoinerStub.rdef
	JoinerStrings.cpp
	JoinerStub.cpp
	JoinerWindow.cpp
	../FileJoiner/Joiner.cpp
	../Widgets/BevelView.cpp
)

target_link_libraries(FileJoinerStub "be")

if(HAIKU_ENABLE_I18N STREQUAL "ON")
	target_link_libraries(FileJoinerStub "localestub")
endif()
