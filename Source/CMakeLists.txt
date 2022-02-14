cmake_minimum_required(VERSION 3.19)

project(BEEZER)

set(CMAKE_MODULE_PATH "${BEEZER_SOURCE_DIR}/CMakeModules")

include(UseHaiku)

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

option(USE_CLANG "Enable building with clang instead of gcc" OFF)
if(USE_CLANG)
	set(CMAKE_CXX_COMPILER clang++)
	set(CMAKE_C_COMPILER clang)
	add_compile_options(-fPIC)
endif()

add_subdirectory(ColumnListView)

option(STRICT_WARNINGS "Compile with extra warnings and errors" ON)
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 2.96 AND STRICT_WARNINGS)
	add_compile_options(-Wall -Wextra -Wshadow -Werror)
	if(USE_CLANG)
		# clang finds a lot more problems than gcc, disable some of the warnings
		# set these here instead of earlier, they must come after -Wall, -Werror, etc...
		add_compile_options(-Wno-overloaded-virtual -Wno-unused-private-field
							-Wno-return-type-c-linkage -Wno-unused-const-variable
							-Wno-deprecated-register)
	endif()
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

option(BUILD_USERGUIDE "Use sphinx to generate the user guide" ON)
if(BUILD_USERGUIDE)
	option(SPHINX_VERBOSE "Enable verbose sphinx output when building the user guide" ON)
	option(SPHINX_FULL_REBUILD "Always copy static files and images when building the user guide" OFF)
	find_package(Sphinx)
	if (Sphinx_FOUND)
		add_subdirectory(../Docs/UserGuide ${PROJECT_BINARY_DIR}/UserGuide)
	else()
		message(WARNING "Sphinx was not found.  You will not be able to build the user guide.")
	endif()
else()
	message("Generating the user guide is disabled.")
endif()

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${BEEZER_BUILD_DIR}/UserGuide")
