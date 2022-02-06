#[[
   Copyright (c) 2020-2022 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

find_path(WLIBC_INCLUDE_DIR NAMES wlibc-macros.h)
find_library(WLIBC_LIBRARY NAMES wlibc NAMES_PER_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WLIBC REQUIRED_VARS WLIBC_INCLUDE_DIR WLIBC_LIBRARY)

if(WLIBC_FOUND)
	set(WLIBC_INCLUDE_DIRS ${WLIBC_INCLUDE_DIR})
	set(WLIBC_LIBRARIES ${WLIBC_LIBRARY})
	if(NOT TARGET WLIBC::WLIBC)
	add_library(WLIBC::WLIBC UNKNOWN IMPORTED)
	set_target_properties(WLIBC::WLIBC PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${WLIBC_INCLUDE_DIRS}
		INTERFACE_LINK_LIBRARIES ${WLIBC_LIBRARIES}
		INTERFACE_LINK_OPTIONS -Wl,-entry:wmainCRTStartup #TODO support MSVC
		IMPORTED_LOCATION ${WLIBC_LIBRARIES})
	endif()
endif()
