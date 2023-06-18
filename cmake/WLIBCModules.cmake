#[[
   Copyright (c) 2020-2023 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

include_guard(GLOBAL)

# Global variable for enabled modules
set(wlibc_enabled_modules)

# Macro for adding modules
macro(wlibc_add_module ...)
	foreach(module ${ARGV})
		list(APPEND wlibc_enabled_modules ${module})
		if(module MATCHES "sys.")
			string(REPLACE "sys." "sys/" module ${module})
		endif()
		add_subdirectory(${module})
	endforeach()
endmacro()


# Function for defining tests
function(wlibc_module ...)
	set(wlibc_module_options)
	set(wlibc_one_value_keywords MODULE)
	set(wlibc_multi_value keywords SOURCES HEADERS)
	cmake_parse_arguments(WLIBC_MODULE_ARG "${wlibc_module_options}" "${wlibc_one_value_keywords}" "${wlibc_multi_value}" ${ARGV})
	add_library(${WLIBC_MODULE_ARG_MODULE} OBJECT ${WLIBC_MODULE_ARG_SOURCES})
	foreach(header ${WLIBC_MODULE_ARG_HEADERS})
		if(header MATCHES "sys/")
			install(FILES ${CMAKE_SOURCE_DIR}/include/${header} DESTINATION include/sys)
		else()
			install(FILES ${CMAKE_SOURCE_DIR}/include/${header} DESTINATION include)
		endif()
	endforeach()
endfunction()
