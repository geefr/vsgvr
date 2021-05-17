cmake_minimum_required(VERSION 3.16)

project(VSG_VR_Test)

if( NOT CMAKE_BUILD_TYPE AND NOT MSVC )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
  set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "MinSizeRel" "RelWithDebInfo" )
endif()

# Compiler/Tool requirements
set( CMAKE_CXX_STANDARD 17 )
set(OpenGL_GL_PREFERENCE GLVND)

#if( NOT MSVC )
#  set( CMAKE_CXX_FLAGS "-Werror=all -Wno-error=unused-variable" )
#endif()

# Package/System requirements
find_package(vsg)

# TODO: Paths hardcoded for now
if( NOT OPENVR_ROOT )
  set( OPENVR_ROOT "" CACHE PATH "Root directory of OpenVR SDK" FORCE )
  message( FATAL_ERROR "VR Support enabled but OPENVR_ROOT not set")
endif()

if( MSVC )
  set( OPENVR_LIB "openvr_api64" )
else()
  set( OPENVR_LIB "openvr_api" )
endif()
#####################

add_subdirectory( vr )

add_executable( ${PROJECT_NAME} 
	main.cpp
       	)
target_link_libraries( ${PROJECT_NAME} vsg::vsg vrhelp )

