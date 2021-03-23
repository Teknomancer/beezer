set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BEEZER_BUILD_DIR})
set(HAIKU_CATALOG_BUILD_DIR ${BEEZER_BUILD_DIR})

include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/Dialogs
		${CMAKE_CURRENT_SOURCE_DIR}/FSUtils
		${CMAKE_CURRENT_SOURCE_DIR}/RegExString
		${CMAKE_CURRENT_SOURCE_DIR}/FileSplitterWindow
		${CMAKE_CURRENT_SOURCE_DIR}/FileSplitter
		${CMAKE_CURRENT_SOURCE_DIR}/FileJoiner
		${CMAKE_CURRENT_SOURCE_DIR}/PrefsWindow
		${CMAKE_CURRENT_SOURCE_DIR}/SplitView
		${CMAKE_CURRENT_SOURCE_DIR}/SplitPane
		${CMAKE_CURRENT_SOURCE_DIR}/URLView
		${CMAKE_CURRENT_SOURCE_DIR}/Widgets
)

if(HAIKU_ENABLE_I18N STREQUAL "ON")
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
		Widgets/BarberPole.cpp
		Beezer.cpp
		Widgets/BeezerListView.cpp
		Widgets/BeezerStringView.cpp
		Widgets/BevelView.cpp
		Widgets/BitmapMenuItem.cpp
		BitmapPool.cpp
		CommentWindow.cpp
		Widgets/ImageButton.cpp
		Widgets/InfoBar.cpp
		Dialogs/InputAlert.cpp
		LangStrings.cpp
		LocalUtils.cpp
		Widgets/LogTextView.cpp
		LogWindow.cpp
		MainMenu.cpp
		MainWindow.cpp
		Preferences.cpp
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
		ProgressWindow.cpp
		RecentMgr.cpp
		RuleMgr.cpp
		SearchWindow.cpp
		Dialogs/SelectDirPanel.cpp
		StartupWindow.cpp
		Widgets/StaticBitmapView.cpp
		StatusWindow.cpp
		Widgets/ToolBar.cpp
		WindowMgr.cpp
		../AppUtils/AppUtils.cpp
		../Archiver/Archiver.cpp
		../ArchiveEntry/ArchiveEntry.cpp
		../HashTable/HashTable.cpp
		../ListEntry/ListEntry.cpp
		../PipeMgr/PipeMgr.cpp
		SplitPane/SplitPane.cpp
		URLView/URLView.cpp
		RegExString/RegExString.cpp
		FileSplitter/FileSplitterWindow.cpp
		FileJoiner/FileJoinerWindow.cpp
		FileJoiner/Joiner.cpp
#		FileJoinerStub/JoinerWindow.cpp
		FSUtils/FSUtils.cpp
)

target_link_libraries(Beezer "be" "tracker" "translation" "CLV")

if(HAIKU_ENABLE_I18N STREQUAL "ON")
	target_link_libraries(Beezer "localestub")
endif()

add_subdirectory(TrackerAddOn)
