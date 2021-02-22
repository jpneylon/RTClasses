# ======================================================================== #
# Copyright 2021 Jack Neylon                                               #
#                                                                          #
# ======================================================================== #

cmake_minimum_required(VERSION 3.0)
project(rtclasses VERSION 1.0.1 DESCRIPTION "DICOMRT I/O Wrappers for DCMTK (C++)")

set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

find_package(DCMTK REQUIRED)
include(GNUInstallDirs)

add_library(${PROJECT_NAME}
  src/rtimages.cpp
	src/rtdose.cpp
	src/rtplan.cpp
	src/rtstruct.cpp
	src/rtrigidreg.cpp
	src/rtdir.cpp
	src/rtcinemr.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION 1)

target_link_libraries(${PROJECT_NAME} ${DCMTK_LIBRARIES})

include_directories(${DCMTK_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}/src)

install(TARGETS ${PROJECT_NAME} EXPORT rtclassesConfig
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(EXPORT rtclassesConfig DESTINATION share/rtclasses/cmake)

export(TARGETS ${PROJECT_NAME} FILE rtclassesConfig.cmake)


