set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BEEZER_BUILD_DIR}/FileJoinerStub)
set(HAIKU_CATALOG_BUILD_DIR ${BEEZER_BUILD_DIR}/FileJoinerStub)

include_directories(
	../Beezer
	../Beezer/FileJoiner
	../Beezer/Widgets
)

if(HAIKU_ENABLE_I18N)
	set("FileJoinerStub-APP_MIME_SIG" "x-vnd.Ram-FileJoiner")
	set("FileJoinerStub-LOCALES" "en")
endif()

haiku_add_executable(FileJoinerStub
	FileJoinerStub.rdef
	JoinerStrings.cpp
	JoinerStub.cpp
	JoinerWindow.cpp
	../Beezer/FileJoiner/Joiner.cpp
	../Beezer/Widgets/BevelView.cpp
)

target_link_libraries(FileJoinerStub "be")

if(HAIKU_ENABLE_I18N)
	target_link_libraries(FileJoinerStub "localestub")
endif()
