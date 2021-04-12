cmake_minimum_required(VERSION 3.19)

project(BEEZER)

set(CMAKE_MODULE_PATH "${BEEZER_SOURCE_DIR}/CMakeModules")

include(UseHaiku)

option(USE_CLANG "Enable building with clang instead of gcc" OFF)
if(USE_CLANG)
	set(CMAKE_CXX_COMPILER clang++)
	set(CMAKE_C_COMPILER clang)
	add_compile_options(-fPIC)
endif()

set(BEEZER_BUILD_DIR ${BEEZER_BINARY_DIR}/build)

set(BEEZER_BUILD_ADDONS_DIR ${BEEZER_BUILD_DIR}/add-ons)

if(HAIKU_ENABLE_I18N)
	set(HAIKU_CATALOG_BUILD_DIR ${BEEZER_BUILD_DIR})
endif()

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
		)

add_subdirectory(ColumnListView)

if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 2.96)
	add_compile_options(-Wall -Wextra -Wshadow)
endif()

add_subdirectory(Beezer)
add_subdirectory(TrackerAddOn)
add_subdirectory(FileJoinerStub)

add_subdirectory(ArjArchiver)
add_subdirectory(BZipArchiver)
add_subdirectory(GZipArchiver)
add_subdirectory(LhaArchiver)
add_subdirectory(RarArchiver)
add_subdirectory(TarArchiver)
add_subdirectory(XzArchiver)
add_subdirectory(z7Archiver)
add_subdirectory(ZipArchiver)
add_subdirectory(ZstdArchiver)

add_custom_target("symlink-docs-to-build-dir"
	ALL
	COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/HelpManual" "${BEEZER_BUILD_DIR}/docs"
	SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/HelpManual"
	VERBATIM)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${BEEZER_BUILD_DIR}/docs")
