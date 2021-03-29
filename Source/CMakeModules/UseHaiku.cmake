#
# - Haiku module for CMake
#

#allow use of LOCATION property
cmake_policy(SET CMP0026 OLD)


#
# Use the standard non-packaged directory if no prefix was given to cmake
#
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	execute_process(COMMAND finddir B_USER_NONPACKAGED_DIRECTORY OUTPUT_VARIABLE B_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(CMAKE_INSTALL_PREFIX "${B_PREFIX}" CACHE PATH "Default non-packaged install path" FORCE)
endif()


#
# Find the proper stdc++ library
#
find_library(LIBSTDCPP NAMES "stdc++" "stdc++.r4")


#
# Add an option to control localization support
#
option(HAIKU_ENABLE_I18N "Enable Haiku localization support")

if(HAIKU_ENABLE_I18N STREQUAL "ON")
	add_custom_target("catkeys")
	add_custom_target("catalogs")
	add_custom_target("bindcatalogs")
	add_custom_target("catalogsinstall"
	  DEPENDS "catalogs"
	  COMMAND "${CMAKE_COMMAND}" "-DCMAKE_INSTALL_COMPONENT=locales" "-P" "${CMAKE_BINARY_DIR}/cmake_install.cmake"
	)
	#TODO check if already set manually
	set(CMAKE_INSTALL_LOCALEDIR "data/locale")
endif()

#
# Override the default add_executable() command and add our own.
#
function(add_executable TARGET)

	foreach(arg ${ARGN})
		if (${arg} MATCHES ".*rdef$")
			list(APPEND rdeflist ${arg})
		elseif(${arg} MATCHES ".*rsrc$")
			list(APPEND rsrclist "${CMAKE_CURRENT_SOURCE_DIR}/${arg}")
		else()
			list(APPEND REAL_SOURCES ${arg})
		endif()
	endforeach()

	# Call the original function with the filtered source list.
	_add_executable(${TARGET} ${REAL_SOURCES})

	if(HAIKU_ENABLE_I18N STREQUAL "ON")
		if(NOT DEFINED "${TARGET}-APP_MIME_SIG")
			message(WARNING "No APP_MIME_SIG property for ${TARGET}. Using 'application/x-vnd.Foo-Bar'")
			set("${TARGET}-APP_MIME_SIG" "application/x-vnd.Foo-Bar")
		endif()

		haiku_generate_base_catkeys(${TARGET} ${REAL_SOURCES})
		if(NOT DEFINED "${TARGET}-LOCALES")
			message(WARNING "No LOCALES property for ${TARGET}. Using 'en'")
			set("${TARGET}-LOCALES" "en")
		endif()

		haiku_compile_catalogs(${TARGET} "${${TARGET}-LOCALES}")
		haiku_bind_catalogs(${TARGET} "${${TARGET}-LOCALES}")
		haiku_install_catalogs(${TARGET} "${${TARGET}-LOCALES}")

		add_compile_definitions("HAIKU_ENABLE_I18N")
	endif()

	# Rdef targets must be added after the main target has been created with _add_executable()
	foreach(rdef ${rdeflist})
		haiku_compile_resource_def(${rdef} rsrcpath)
		list(APPEND rsrclist ${rsrcpath})
	endforeach()

	# make sure rsrclist isn't empty
	foreach(rsrc ${rsrclist})
		get_filename_component(shortname ${rsrc} NAME)
		add_custom_command(
			TARGET ${TARGET}
			POST_BUILD
			COMMAND "xres" "-o" "$<TARGET_FILE:${TARGET}>" "${rsrc}"
			COMMENT "Merging resources from ${shortname} into ${TARGET}")
	endforeach()

	add_custom_command(
		TARGET ${TARGET}
		POST_BUILD
		COMMAND "mimeset" "-f" "$<TARGET_FILE:${TARGET}>"
		COMMENT "Setting mimetype for ${TARGET}")

endfunction()

#
# Compile a resource definition file(.rdef) to a resource file(.rsrc)
#
function(haiku_compile_resource_def RDEF_SOURCE)

	get_filename_component(rdefpath ${RDEF_SOURCE} ABSOLUTE)
	get_filename_component(basename ${RDEF_SOURCE} NAME_WE)

	set(rsrcfile "${basename}.rsrc")
	set(rsrcpath "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${rsrcfile}.dir/${rsrcfile}")

	get_target_property(TARGET_PATH ${TARGET} LOCATION)
	#FIXME we need the path to the target without generating a dependency on it
	#set(TARGET_PATH $<TARGET_FILE:${TARGET}>)

	add_custom_command(
		OUTPUT ${rsrcpath}
		COMMAND "rc" "-o" "${rsrcfile}" "${rdefpath}"
		COMMAND "${CMAKE_COMMAND}" "-E" "remove" "-f" "${TARGET_PATH}"
		DEPENDS ${rdefpath}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${rsrcfile}.dir
		COMMENT "Compiling resource ${rsrcfile}")

	add_custom_target(${rsrcfile} DEPENDS ${rsrcpath})

	set_source_files_properties(${rsrcfile} PROPERTIES GENERATED TRUE)

	set(rsrcpath "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${rsrcfile}.dir/${rsrcfile}" PARENT_SCOPE)
	add_dependencies(${TARGET} ${rsrcfile})

endfunction()

#
#	Regenerate the main locales/en.catkeys file for a target
#
function(haiku_generate_base_catkeys TARGET)

	haiku_get_app_mime_subtype("${${TARGET}-APP_MIME_SIG}" SUBTYPE)
	add_custom_target(
		"${TARGET}-generate-en.catkeys"
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/locales
		COMMAND sh -c "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -I$<JOIN:$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>, -I> -DB_COLLECTING_CATKEYS -DHAIKU_ENABLE_I18N -E ${CMAKE_CURRENT_SOURCE_DIR}/$<JOIN:$<TARGET_PROPERTY:${TARGET},SOURCES>, ${CMAKE_CURRENT_SOURCE_DIR}/> >${TARGET}.cpp.i"
		COMMAND sh -c "grep -av '^#' ${TARGET}.cpp.i >${TARGET}.cpp.ck"
		COMMAND collectcatkeys -pvw -s ${SUBTYPE} -o $<TARGET_PROPERTY:${TARGET},SOURCE_DIR>/locales/en.catkeys ${TARGET}.cpp.ck
		COMMAND ${CMAKE_COMMAND} -E rm ${TARGET}.cpp.i ${TARGET}.cpp.ck
		DEPENDS $<TARGET_PROPERTY:${TARGET},SOURCES>
		VERBATIM
		COMMAND_EXPAND_LISTS
		COMMENT "Generating locales/en.catkeys for ${TARGET}")

	add_dependencies("catkeys" "${TARGET}-generate-en.catkeys")

endfunction()

#
#	Compile catkeys files into binary catalog files for a target
#
function(haiku_compile_catalogs TARGET)
	haiku_get_app_mime_subtype("${${TARGET}-APP_MIME_SIG}" SUBTYPE)

	if(DEFINED HAIKU_CATALOG_BUILD_DIR)
		set(catalogspath "${HAIKU_CATALOG_BUILD_DIR}/locale/catalogs/${SUBTYPE}")
	else()
		set(catalogspath "${CMAKE_CURRENT_BINARY_DIR}/locale/catalogs/${SUBTYPE}")
	endif()

	foreach(lang ${ARGN})
		set(catalogoutput "${catalogspath}/${lang}.catalog")
		set(catkeyspath "${CMAKE_CURRENT_SOURCE_DIR}/locales/${lang}.catkeys")

		add_custom_command(
			OUTPUT ${catalogoutput}
			COMMAND "${CMAKE_COMMAND}" "-E" "make_directory" "${catalogspath}"
			COMMAND "linkcatkeys" "-o" "${catalogoutput}" "-s" "${${TARGET}-APP_MIME_SIG}" "-l" "${lang}" "${catkeyspath}"
			DEPENDS ${catkeyspath}
			COMMENT "Compiling ${lang}.catalog for ${TARGET}")

		add_custom_target("${TARGET}-${lang}.catalog" DEPENDS ${catalogoutput})
		add_dependencies("catalogs" "${TARGET}-${lang}.catalog")
	endforeach()

endfunction()

#
#	Compile and bind catkeys files directly into the target executable as resources
#
function(haiku_bind_catalogs TARGET)

	foreach(lang ${ARGN})
		set(catkeyspath "${CMAKE_CURRENT_SOURCE_DIR}/locales/${lang}.catkeys")

		add_custom_target(
			"${TARGET}-bind-${lang}.catalog"
			COMMAND "linkcatkeys" "-o" "$<TARGET_FILE:${TARGET}>" "-s" "${${TARGET}-APP_MIME_SIG}" "-tr" "-l" "${lang}" "${catkeyspath}"
			DEPENDS ${catkeyspath} ${TARGET}
			COMMENT "Binding ${lang}.catalog to ${TARGET}")

		add_dependencies("bindcatalogs" "${TARGET}-bind-${lang}.catalog")
	endforeach()

endfunction()

#
# Generate install rules for catalog files
#
function(haiku_install_catalogs TARGET)
	haiku_get_app_mime_subtype("${${TARGET}-APP_MIME_SIG}" SUBTYPE)

	if(DEFINED HAIKU_CATALOG_BUILD_DIR)
		set(catalogspath "${HAIKU_CATALOG_BUILD_DIR}/locale/catalogs/${SUBTYPE}")
	else()
		set(catalogspath "${CMAKE_CURRENT_BINARY_DIR}/locale/catalogs/${SUBTYPE}")
	endif()

	foreach(lang ${ARGN})
		install(FILES "${catalogspath}/${lang}.catalog"
			DESTINATION "${CMAKE_INSTALL_LOCALEDIR}/catalogs/${SUBTYPE}"
			COMPONENT "locales"
			EXCLUDE_FROM_ALL)
	endforeach()
endfunction()

#
# Split mimetype at last / if needed
#
function(haiku_get_app_mime_subtype APP_MIME_SIG OUTVAR)
	# ensure that we have a shortened mimetype without the application/ prefix
	# find the last / and split the mime string
	string(FIND "${${TARGET}-APP_MIME_SIG}" "/" SUBPOS REVERSE)
	if("${SUBPOS}" EQUAL "-1")
		set("${OUTVAR}" "${${TARGET}-APP_MIME_SIG}" PARENT_SCOPE)
		return()
	endif()
	math(EXPR SUBPOS "${SUBPOS}+1")
	string(SUBSTRING "${${TARGET}-APP_MIME_SIG}" "${SUBPOS}+1" "-1" SUBTYPE)
	set("${OUTVAR}" "${SUBTYPE}" PARENT_SCOPE)
endfunction()
