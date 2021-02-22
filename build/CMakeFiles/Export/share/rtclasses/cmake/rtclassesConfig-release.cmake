#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "rtclasses" for configuration "Release"
set_property(TARGET rtclasses APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(rtclasses PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/librtclasses.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS rtclasses )
list(APPEND _IMPORT_CHECK_FILES_FOR_rtclasses "${_IMPORT_PREFIX}/lib/librtclasses.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
