cmake_minimum_required(VERSION 3.14)

project(VSGVR)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(GNUInstallDirs)

include(FetchContent)

if( NOT CMAKE_BUILD_TYPE AND NOT MSVC )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
  set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "MinSizeRel" "RelWithDebInfo" )
endif()

# Compiler/Tool requirements
set( CMAKE_CXX_STANDARD 17 )
set(OpenGL_GL_PREFERENCE GLVND)

# set( FORMAT_SKIP_CMAKE ON )
# add_subdirectory(deps/Format.cmake)

# Package/System requirements
find_package(vsg REQUIRED)
find_package(Vulkan REQUIRED)

# TODO: openvr paths are hardcoded
set( OPENVR_ROOT ${PROJECT_SOURCE_DIR}/deps/openvr )
set( OPENVR_LIB "openvr_api" )
if( MSVC )
  set( OPENVR_BINDIR ${OPENVR_ROOT}/bin/win64 )
  set( OPENVR_LIBDIR ${OPENVR_ROOT}/lib/win64 )
else()
  set( OPENVR_BINDIR ${OPENVR_ROOT}/bin/linux64 )
  set( OPENVR_LIBDIR ${OPENVR_ROOT}/lib/linux64 )
endif()

#####################

add_subdirectory( vsgvr )

add_executable( example_vr example_vr.cpp )
target_link_libraries( example_vr vsg::vsg vsgvr )
target_include_directories( example_vr PRIVATE 
  ${OPENVR_ROOT}/headers
  vsgvr/include )
if( MSVC )
  set_target_properties(example_vr PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${OPENVR_BINDIR}")
endif()

# Copy models into the build dir
configure_file(models/world/world.vsgt ${CMAKE_CURRENT_BINARY_DIR}/world.vsgt COPYONLY)
configure_file(models/controller/controller.vsgt ${CMAKE_CURRENT_BINARY_DIR}/controller.vsgt COPYONLY)
configure_file(models/controller/controller2.vsgt ${CMAKE_CURRENT_BINARY_DIR}/controller2.vsgt COPYONLY)

add_subdirectory( tests )
