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
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)


#
# Override the default add_executable() command and add our own.
#
function(ADD_EXECUTABLE TARGET)

	foreach(arg ${ARGN})
		if (${arg} MATCHES ".*rdef$")
			list(APPEND rdeflist ${arg})
		elseif(${arg} MATCHES ".*rsrc$")
			list(APPEND rsrclist "${CMAKE_CURRENT_SOURCE_DIR}/${arg}")
		else(${arg} MATCHES ".*rdef$")
			list(APPEND REAL_SOURCES ${arg})
		endif(${arg} MATCHES ".*rdef$")
	endforeach(arg)

	# Call the original function with the filtered source list.
	_add_executable(${TARGET} ${REAL_SOURCES})

	# Rdef targets must be added after the main target has been created with _add_executable()
	foreach(rdef ${rdeflist})
		haiku_compile_resource_def(${rdef} rsrcpath)
		list(APPEND rsrclist ${rsrcpath})
	endforeach(rdef ${rdeflist})

	# make sure rsrclist isn't empty
	foreach(rsrc ${rsrclist})
		get_filename_component(shortname ${rsrc} NAME)
		add_custom_command(
			TARGET ${TARGET}
			POST_BUILD
			COMMAND "xres" "-o" "$<TARGET_FILE:${TARGET}>" ${rsrc}
			COMMENT "Merging resources from ${shortname} into ${TARGET}")
	endforeach(rsrc ${rsrclist})

	add_custom_command(
		TARGET ${TARGET}
		POST_BUILD
		COMMAND "mimeset" "-f" "$<TARGET_FILE:${TARGET}>"
		COMMENT "Setting mimetype for ${TARGET}")

endfunction(ADD_EXECUTABLE)

#
# Compile a resource definition file(.rdef) to a resource file(.rsrc)
#
function(HAIKU_COMPILE_RESOURCE_DEF RDEF_SOURCE)

	get_filename_component(rdefpath ${RDEF_SOURCE} ABSOLUTE)
	get_filename_component(basename ${RDEF_SOURCE} NAME_WE)

	set(rsrcfile "${basename}.rsrc")
	set(rsrcpath "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${rsrcfile}.dir/${rsrcfile}")

	get_target_property(TARGET_PATH ${TARGET} LOCATION)

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

endfunction(HAIKU_COMPILE_RESOURCE_DEF)
