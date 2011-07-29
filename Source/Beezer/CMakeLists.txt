set(EXECUTABLE_OUTPUT_PATH ${BEEZER_BUILD_DIR})

include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/FSUtils
		${CMAKE_CURRENT_SOURCE_DIR}/RegExString
		${CMAKE_CURRENT_SOURCE_DIR}/FileSplitterWindow
		${CMAKE_CURRENT_SOURCE_DIR}/FileSplitter
		${CMAKE_CURRENT_SOURCE_DIR}/FileJoiner
		${CMAKE_CURRENT_SOURCE_DIR}/SplitView
		${CMAKE_CURRENT_SOURCE_DIR}/SplitPane
		${CMAKE_CURRENT_SOURCE_DIR}/URLView
)

add_executable(Beezer
		Beezer.rdef
		AboutWindow.cpp
		AddOnWindow.cpp
		ArchiveRep.cpp
		ArchiverMgr.cpp
		ArkInfoWindow.cpp
		AuthorWindow.cpp
		BarberPole.cpp
		Beezer.cpp
		BeezerListView.cpp
		BeezerStringView.cpp
		BevelView.cpp
		BitmapMenuItem.cpp
		BitmapPool.cpp
		CommentWindow.cpp
		ImageButton.cpp
		InfoBar.cpp
		InputAlert.cpp
		LangStrings.cpp
		LocalUtils.cpp
		LogTextView.cpp
		LogWindow.cpp
		MainMenu.cpp
		MainWindow.cpp
		Preferences.cpp
		PrefsListItem.cpp
		PrefsView.cpp
		PrefsViewAdd.cpp
		PrefsViewExtract.cpp
		PrefsViewInterface.cpp
		PrefsViewMisc.cpp
		PrefsViewPaths.cpp
		PrefsViewRecent.cpp
		PrefsViewState.cpp
		PrefsViewWindows.cpp
		PrefsWindow.cpp
		ProgressWindow.cpp
		RecentMgr.cpp
		RuleMgr.cpp
		SearchWindow.cpp
		SelectDirPanel.cpp
		StartupWindow.cpp
		StaticBitmapView.cpp
		StatusWindow.cpp
		ToolBar.cpp
		WindowMgr.cpp
		../AppUtils/AppUtils.cpp
		../Archiver/Archiver.cpp
		../ArchiveEntry/ArchiveEntry.cpp
		../BubbleHelper-1.2/BubbleHelper.cpp
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

target_link_libraries(Beezer "be" "tracker" "translation" CLV)

add_custom_command(TARGET Beezer POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/workers ${BEEZER_BUILD_DIR}/workers
					COMMENT "Copying workers to build directory")

add_custom_command(TARGET Beezer POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/settings ${BEEZER_BUILD_DIR}/settings
					COMMENT "Copying settings to build directory")