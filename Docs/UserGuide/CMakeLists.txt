
set(SPHINX_ARGS "-W")

if(NOT SPHINX_VERBOSE)
	list(APPEND SPHINX_ARGS "-q")
endif()

if(SPHINX_FULL_REBUILD)
	list(APPEND SPHINX_ARGS "-a")
endif()

set(USERGUIDE_SOURCES
	custom.css
	AddingFiles.rst
	AddonsWorkers.rst
	ArchiveInfo.rst
	ArchiveWindow.rst
	Contact.rst
	Conventions.rst
	CreatingArchives.rst
	CreatingFolders.rst
	Credits.rst
	DeletingArchives.rst
	DeletingFiles.rst
	EditingComments.rst
	ExtractingFiles.rst
	Features.rst
	FileTypeRules.rst
	index.rst
	Installation.rst
	Introduction.rst
	KnownBugs.rst
	License.rst
	Preferences.rst
	Requirements.rst
	SearchingArchives.rst
	TestingArchives.rst
	VersionHistory.rst
	ViewingFiles.rst
	WelcomeWindow.rst
	addons/7Zip.rst
	addons/Arj.rst
	addons/Bzip2.rst
	addons/Gzip.rst
	addons/Hpkg.rst
	addons/Lha.rst
	addons/Rar.rst
	addons/Xz.rst
	addons/Zip.rst
	addons/Zstd.rst
	tools/FileJoiner.rst
	tools/FileSplitter.rst
	tools/QuickCreate.rst
)

#TODO remove extra files that the theme installs
add_custom_command(
	OUTPUT ${BEEZER_BUILD_DIR}/UserGuide/index.html
	COMMAND ${SPHINX_BUILD_EXE} ${SPHINX_ARGS} -b html -d ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${BEEZER_BUILD_DIR}/UserGuide
	MAIN_DEPENDENCY conf.py
	DEPENDS ${USERGUIDE_SOURCES}
	COMMENT "Building user guide"
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
)

add_custom_target("UserGuide" ALL DEPENDS ${BEEZER_BUILD_DIR}/UserGuide/index.html)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES
	${PROJECT_BINARY_DIR}/UserGuide
	${CMAKE_CURRENT_BINARY_DIR}/environment.pickle
	${CMAKE_CURRENT_BINARY_DIR}/index.doctree
)
