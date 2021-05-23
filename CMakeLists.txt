cmake_minimum_required(VERSION 3.14)

project(VSG_VR_Test)

include(FetchContent)

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
find_package(vsg REQUIRED)
find_package(Vulkan REQUIRED)

# TODO: Paths hardcoded for now
if( NOT OPENVR_ROOT )
  set( OPENVR_ROOT "" CACHE PATH "Root directory of OpenVR SDK" FORCE )
  message( WARNING "VR Support enabled but OPENVR_ROOT not set")
endif()

if( MSVC )
  set( OPENVR_LIB "openvr_api64" )
else()
  set( OPENVR_LIB "openvr_api" )
endif()
#####################

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

#####################

add_subdirectory( vr )


add_executable( example_vr example_vr.cpp )
target_link_libraries( example_vr vsg::vsg vrhelp fmt::fmt )
target_link_directories( example_vr PRIVATE ${OPENVR_ROOT}/lib/linux64 )
target_include_directories( example_vr PRIVATE ${OPENVR_ROOT}/headers )

# Copy models into the build dir
configure_file(models/world/world.vsgt ${CMAKE_CURRENT_BINARY_DIR}/world.vsgt COPYONLY)
configure_file(models/controller/controller.vsgt ${CMAKE_CURRENT_BINARY_DIR}/controller.vsgt COPYONLY)

add_subdirectory( tests )
