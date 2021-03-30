include(CMakeDependentOption)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BEEZER_BUILD_DIR})
set(HAIKU_CATALOG_BUILD_DIR ${BEEZER_BUILD_DIR})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/Dialogs
	${CMAKE_CURRENT_SOURCE_DIR}/FSUtils
	${CMAKE_CURRENT_SOURCE_DIR}/RegExString
	${CMAKE_CURRENT_SOURCE_DIR}/FileSplitter
	${CMAKE_CURRENT_SOURCE_DIR}/FileJoiner
	${CMAKE_CURRENT_SOURCE_DIR}/PrefsWindow
	${CMAKE_CURRENT_SOURCE_DIR}/SplitPane
	${CMAKE_CURRENT_SOURCE_DIR}/Widgets
)

if(HAIKU_ENABLE_I18N)
	set("Beezer-APP_MIME_SIG" "x-vnd.Ram-Beezer")
	set("Beezer-LOCALES" "en")
endif()

add_executable(Beezer
	Beezer.rdef
	AboutWindow.cpp
	AddOnWindow.cpp
	ArchiveRep.cpp
	ArchiverMgr.cpp
	ArkInfoWindow.cpp
	BeezerApp.cpp
	BitmapPool.cpp
	CommentWindow.cpp
	LocalUtils.cpp
	LogWindow.cpp
	MainMenu.cpp
	MainWindow.cpp
	Preferences.cpp
	ProgressWindow.cpp
	RecentMgr.cpp
	RuleMgr.cpp
	SearchWindow.cpp
	StartupWindow.cpp
	StatusWindow.cpp
	WindowMgr.cpp
	../AppUtils/AppUtils.cpp
	../Archiver/Archiver.cpp
	../ArchiveEntry/ArchiveEntry.cpp
	../HashTable/HashTable.cpp
	../ListEntry/ListEntry.cpp
	../PipeMgr/PipeMgr.cpp
	Dialogs/InputAlert.cpp
	Dialogs/SelectDirPanel.cpp
	FileJoiner/FileJoinerWindow.cpp
	FileJoiner/Joiner.cpp
	FileSplitter/FileSplitterWindow.cpp
	FSUtils/FSUtils.cpp
	PrefsWindow/PrefsListItem.cpp
	PrefsWindow/PrefsView.cpp
	PrefsWindow/PrefsViewAdd.cpp
	PrefsWindow/PrefsViewExtract.cpp
	PrefsWindow/PrefsViewInterface.cpp
	PrefsWindow/PrefsViewMisc.cpp
	PrefsWindow/PrefsViewPaths.cpp
	PrefsWindow/PrefsViewRecent.cpp
	PrefsWindow/PrefsViewState.cpp
	PrefsWindow/PrefsViewWindows.cpp
	PrefsWindow/PrefsWindow.cpp
	RegExString/RegExString.cpp
	SplitPane/SplitPane.cpp
	Widgets/BarberPole.cpp
	Widgets/BeezerListView.cpp
	Widgets/BeezerStringView.cpp
	Widgets/BevelView.cpp
	Widgets/BitmapMenuItem.cpp
	Widgets/ImageButton.cpp
	Widgets/InfoBar.cpp
	Widgets/LogTextView.cpp
	Widgets/StaticBitmapView.cpp
	Widgets/ToolBar.cpp
)

target_link_libraries(Beezer "be" "tracker" "translation" "CLV")

if(HAIKU_ENABLE_I18N)
	target_link_libraries(Beezer "localestub")
endif()


option(BEEZER_EXE_ATTR_TAGGING "Enable setting BFS attributes with build information on the executable" ON)
cmake_dependent_option(BEEZER_EXE_ATTR_HOST "Set a BFS attribute with the Haiku version of the build host" ON "BEEZER_EXE_ATTR_TAGGING" OFF)
cmake_dependent_option(BEEZER_EXE_ATTR_GIT "Set a BFS attribute with the git revision" ON "BEEZER_EXE_ATTR_TAGGING" OFF)

if(BEEZER_EXE_ATTR_HOST)
	add_custom_command(
		TARGET Beezer
		POST_BUILD
		COMMAND sh -c "addattr -t string 'BEEZER:HAIKU_VERSION' \"`uname -v -p`\" '$<TARGET_FILE:Beezer>'"
		VERBATIM
		COMMENT "Adding BEEZER:HAIKU_VERSION BFS attribute")
endif()

if(BEEZER_EXE_ATTR_GIT)
	set(BEEZER_OVERRIDE_ATTR_GIT "" CACHE STRING "String to use as the git revision for the BFS attribute")

	if(BEEZER_OVERRIDE_ATTR_GIT)
		set(BEEZER_GIT_REVISION "${BEEZER_OVERRIDE_ATTR_GIT}")
	else()
		find_program(GITEXEPATH git REQUIRED)
		if(GITEXEPATH)
			set(BEEZER_GIT_REVISION "`${GITEXEPATH} describe --tags --dirty --always`")
		else()
			message(WARNING "git command not found and BEEZER_OVERRIDE_ATTR_GIT is not set")
		endif()
	endif()

	if(BEEZER_GIT_REVISION)
		add_custom_command(
			TARGET Beezer
			POST_BUILD
			COMMAND sh -c "addattr -t string 'BEEZER:GIT_REVISION' \"${BEEZER_GIT_REVISION}\" '$<TARGET_FILE:Beezer>'"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			VERBATIM
			COMMENT "Adding BEEZER:GIT_REVISION BFS attribute")
	endif()
endif()
