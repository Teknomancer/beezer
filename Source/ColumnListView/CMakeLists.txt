include_directories(
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/BetterScrollView
		${CMAKE_CURRENT_SOURCE_DIR}/ColumnListView
		${CMAKE_CURRENT_SOURCE_DIR}/Cursors
		${CMAKE_CURRENT_SOURCE_DIR}/NewStrings
		${CMAKE_CURRENT_SOURCE_DIR}/PrefilledBitmap
		${CMAKE_CURRENT_SOURCE_DIR}/ScrollViewCorner
)

add_library(CLV
	BetterScrollView/BetterScrollView.cpp
	ColumnListView/CLVColumn.cpp
	ColumnListView/CLVColumnLabelView.cpp
	ColumnListView/CLVEasyItem.cpp
	ColumnListView/CLVListItem.cpp
	ColumnListView/ColumnListView.cpp
	ColumnListView/ColumnListView_jaf.cpp
	Cursors/Cursors.cpp
	NewStrings/NewStrings.cpp
	PrefilledBitmap/PrefilledBitmap.cpp
	ScrollViewCorner/ScrollViewCorner.cpp
)

#target_link_libraries(Beezer "be" "tracker" "translation")


