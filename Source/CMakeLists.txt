cmake_minimum_required(VERSION 2.8)

project(BEEZER)

#Override the default module naming scheme
#so that we get foo instead of libfoo.so
set(CMAKE_SHARED_MODULE_PREFIX "")
set(CMAKE_SHARED_MODULE_SUFFIX "")

set(CMAKE_MODULE_PATH "${BEEZER_SOURCE_DIR}/CMakeModules")

set(BEEZER_BUILD_DIR ${BEEZER_BINARY_DIR}/build)

set(BEEZER_BUILD_ADDONS_DIR ${BEEZER_BINARY_DIR}/build/add-ons)

include(UseHaiku)

include_directories(
		"${BEEZER_SOURCE_DIR}/Archiver"
		"${BEEZER_SOURCE_DIR}/ArchiveEntry"
		"${BEEZER_SOURCE_DIR}/PipeMgr"
		"${BEEZER_SOURCE_DIR}/Shared"
		"${BEEZER_SOURCE_DIR}/HashTable"
		"${BEEZER_SOURCE_DIR}/ListEntry"
		"${BEEZER_SOURCE_DIR}/ColumnListView"
		"${BEEZER_SOURCE_DIR}/ColumnListView/ColumnListView"
		"${BEEZER_SOURCE_DIR}/ColumnListView/PrefilledBitmap"
		"${BEEZER_SOURCE_DIR}/ColumnListView/BetterScrollView"
		"${BEEZER_SOURCE_DIR}/ColumnListView/ScrollViewCorner"
		"${BEEZER_SOURCE_DIR}/AppUtils"
		"${BEEZER_SOURCE_DIR}/BubbleHelper-1.2"
		"${BEEZER_SOURCE_DIR}/_Deprecated/ListSorter"
		)

add_subdirectory(ColumnListView)

add_subdirectory(ArjArchiver)
add_subdirectory(BZipArchiver)
add_subdirectory(GZipArchiver)
add_subdirectory(LhaArchiver)
add_subdirectory(RarArchiver)
add_subdirectory(TarArchiver)
add_subdirectory(ZipArchiver)
add_subdirectory(z7Archiver)

add_subdirectory(Beezer)

