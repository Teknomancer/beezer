include(CMakeDependentOption)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BEEZER_BUILD_DIR})

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

haiku_add_executable(Beezer
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
	set("Beezer-APP_MIME_SIG" "x-vnd.Ram-Beezer")
	set("Beezer-LOCALES" "en")
	target_link_libraries(Beezer "localestub")
	haiku_add_i18n(Beezer)
endif()

cmake_dependent_option(BEEZER_EXE_ATTR_HOST "Set a BFS attribute with the Haiku version of the build host" ON "HAIKU_ENABLE_TARGET_ATTRS" OFF)
if(BEEZER_EXE_ATTR_HOST)
	haiku_add_target_attr(Beezer "BEEZER:HAIKU_VERSION" "`uname -v -p`")
endif()

cmake_dependent_option(BEEZER_EXE_ATTR_GIT "Set a BFS attribute with the git revision" ON "HAIKU_ENABLE_TARGET_ATTRS" OFF)
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
		haiku_add_target_attr(Beezer "BEEZER:GIT_REVISION" "${BEEZER_GIT_REVISION}")
	endif()
endif()
