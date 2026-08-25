# Makes an installed macOS tree self-contained.
#
# The binaries link against libraries that live outside the install tree, such as
# the ones Homebrew provides. Those paths do not exist on a machine that never
# installed them, so the app only starts on the machine that built it. This copies
# every such library next to the binaries, rewrites the load commands to
# @executable_path and repeats the walk for the copied libraries themselves, since
# they pull in dependencies of their own.
#
# Runs as an install script, so `cmake --install` and `cpack` both benefit.
#
# Libraries under /usr/lib and /System are left alone: those ship with macOS and
# are guaranteed to be present. Paths that are already relative to the bundle
# (@rpath, @executable_path, @loader_path) are likewise skipped, SDL2.framework
# being the notable one.

# Returns the install names ${binary} refers to, minus the leading line that otool
# prints for the file itself.
function(_ja2_get_link_deps binary out_var)
	execute_process(
		COMMAND otool -L "${binary}"
		OUTPUT_VARIABLE _otool_output
		ERROR_VARIABLE _otool_error
		RESULT_VARIABLE _otool_result
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	if(NOT _otool_result EQUAL 0)
		message(FATAL_ERROR "otool -L failed for ${binary}: ${_otool_error}")
	endif()

	string(REPLACE "\n" ";" _lines "${_otool_output}")
	set(_deps "")
	foreach(_line IN LISTS _lines)
		# the dependency lines are indented, the header naming the file is not
		if(NOT _line MATCHES "^[ \t]+")
			continue()
		endif()
		# "\tpath/to/lib.dylib (compatibility version ..., current version ...)"
		if(_line MATCHES "^[ \t]+(.+) \\(compatibility version")
			list(APPEND _deps "${CMAKE_MATCH_1}")
		endif()
	endforeach()

	set(${out_var} "${_deps}" PARENT_SCOPE)
endfunction()

# Editing a Mach-O invalidates its code signature, and arm64 refuses to run an
# incorrectly signed binary, so everything touched gets an ad-hoc signature.
function(_ja2_resign binary)
	execute_process(
		COMMAND codesign --force --sign - "${binary}"
		RESULT_VARIABLE _codesign_result
		ERROR_VARIABLE _codesign_error)
	if(NOT _codesign_result EQUAL 0)
		message(FATAL_ERROR "codesign failed for ${binary}: ${_codesign_error}")
	endif()
endfunction()

function(ja2_bundle_macos_dylibs destination)
	set(_queue "")
	foreach(_name IN LISTS ARGN)
		if(EXISTS "${destination}/${_name}")
			list(APPEND _queue "${destination}/${_name}")
		endif()
	endforeach()

	if(NOT _queue)
		message(WARNING "No binaries to make self-contained in ${destination}")
		return()
	endif()

	set(_touched "${_queue}")
	set(_bundled "")

	while(_queue)
		list(POP_FRONT _queue _binary)
		_ja2_get_link_deps("${_binary}" _deps)

		foreach(_dep IN LISTS _deps)
			if(_dep MATCHES "^@")
				continue() # already resolved relative to the bundle
			endif()
			if(_dep MATCHES "^/usr/lib/" OR _dep MATCHES "^/System/")
				continue() # part of macOS itself
			endif()

			get_filename_component(_dep_name "${_dep}" NAME)
			set(_local "${destination}/${_dep_name}")

			if(NOT _dep_name IN_LIST _bundled)
				if(NOT EXISTS "${_dep}")
					message(FATAL_ERROR "Cannot bundle missing library ${_dep} needed by ${_binary}")
				endif()
				message(STATUS "Bundling ${_dep}")
				file(COPY "${_dep}" DESTINATION "${destination}")
				# Homebrew installs libraries read-only, install_name_tool needs to write
				file(CHMOD "${_local}" PERMISSIONS
					OWNER_READ OWNER_WRITE OWNER_EXECUTE
					GROUP_READ GROUP_EXECUTE
					WORLD_READ WORLD_EXECUTE)
				execute_process(COMMAND install_name_tool -id "@executable_path/${_dep_name}" "${_local}"
					RESULT_VARIABLE _id_result ERROR_VARIABLE _id_error)
				if(NOT _id_result EQUAL 0)
					message(FATAL_ERROR "install_name_tool -id failed for ${_local}: ${_id_error}")
				endif()

				list(APPEND _bundled "${_dep_name}")
				list(APPEND _queue "${_local}")  # its own dependencies still need walking
				list(APPEND _touched "${_local}")
			endif()

			execute_process(COMMAND install_name_tool -change "${_dep}" "@executable_path/${_dep_name}" "${_binary}"
				RESULT_VARIABLE _change_result ERROR_VARIABLE _change_error)
			if(NOT _change_result EQUAL 0)
				message(FATAL_ERROR "install_name_tool -change failed for ${_binary}: ${_change_error}")
			endif()
		endforeach()
	endwhile()

	foreach(_binary IN LISTS _touched)
		_ja2_resign("${_binary}")
	endforeach()

	list(LENGTH _bundled _bundled_count)
	message(STATUS "Made install tree self-contained, bundled ${_bundled_count} librar(y/ies)")
endfunction()
