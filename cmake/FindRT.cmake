# FindRT.cmake - Try to find the RT library
# Once done this will define
#
#  RT_FOUND - System has rt
#  RT_INCLUDE_DIR - The rt include directory
#  RT_LIBRARIES - The libraries needed to use rt
#  RT_DEFINITIONS - Compiler switches required for using rt
#
# Also creates an import target called RT::RT

find_path (RT_INCLUDE_DIR NAMES time.h
  PATHS
  /usr
  /usr/local
  /opt
  PATH_SUFFIXES
)

find_library(RT_LIBRARIES NAMES rt
  PATHS
  /usr
  /usr/local
  /opt
)

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(rt DEFAULT_MSG RT_LIBRARIES RT_INCLUDE_DIR)

mark_as_advanced(RT_INCLUDE_DIR RT_LIBRARIES)

if (NOT TARGET RT::RT)
  add_library(RT::RT INTERFACE IMPORTED)

  set_target_properties(RT::RT PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${RT_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES ${RT_LIBRARIES}
  )
endif()