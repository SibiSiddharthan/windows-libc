#[[
   Copyright (c) 2020-2025 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory for details.
]]

include(SelectLibraryConfigurations)
include(FindPackageHandleStandardArgs)

find_path(WLIBC_INCLUDE_DIR NAMES wlibc.h)
find_library(WLIBC_LIBRARY_RELEASE NAMES wlibc wlibcs NAMES_PER_DIR)
find_library(WLIBC_LIBRARY_DEBUG NAMES wlibcd wlibcds NAMES_PER_DIR)

select_library_configurations(WLIBC)
find_package_handle_standard_args(WLIBC REQUIRED_VARS WLIBC_INCLUDE_DIR WLIBC_LIBRARY)

if(WLIBC_FOUND)
	include_directories(BEFORE ${WLIBC_INCLUDE_DIR})
	set(WLIBC_LIBRARIES ${WLIBC_LIBRARY})
	if(NOT TARGET WLIBC::WLIBC)
		add_library(WLIBC::WLIBC UNKNOWN IMPORTED)
		set_target_properties(WLIBC::WLIBC PROPERTIES INTERFACE_LINK_OPTIONS "LINKER:/entry:wmainCRTStartup")
	endif()
	if(WLIBC_LIBRARY_RELEASE)
		set_target_properties(WLIBC::WLIBC PROPERTIES IMPORTED_CONFIGURATIONS RELEASE)
		set_target_properties(WLIBC::WLIBC PROPERTIES IMPORTED_LOCATION_RELEASE ${WLIBC_LIBRARY_RELEASE})
	endif()
	if(WLIBC_LIBRARY_DEBUG)
		set_target_properties(WLIBC::WLIBC PROPERTIES IMPORTED_CONFIGURATIONS DEBUG)
		set_target_properties(WLIBC::WLIBC PROPERTIES IMPORTED_LOCATION_DEBUG ${WLIBC_LIBRARY_DEBUG})
	endif()
endif()
