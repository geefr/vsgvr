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
#####################

FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 7.1.3
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

#####################

# TODO: openvr paths are hardcoded
set( OPENVR_LIB "openvr_api" )
if( MSVC )
  set( OPENVR_BINDIR ${OPENVR_ROOT}/bin/win64 )
  set( OPENVR_LIBDIR ${OPENVR_ROOT}/lib/win64 )
else()
  set( OPENVR_BINDIR ${OPENVR_ROOT}/bin/linux64 )
  set( OPENVR_LIBDIR ${OPENVR_ROOT}/lib/linux64 )
endif()

#####################

add_subdirectory( vr )

add_executable( example_vr example_vr.cpp )
target_link_libraries( example_vr vsg::vsg vrhelp fmt::fmt )
target_include_directories( example_vr PRIVATE ${OPENVR_ROOT}/headers )
if( MSVC )
  set_target_properties(example_vr PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${OPENVR_BINDIR}")
endif()

add_executable( example_novr example_novr.cpp )
target_link_libraries( example_novr vsg::vsg vrhelp )
target_include_directories( example_novr PRIVATE ${OPENVR_ROOT}/headers )

add_subdirectory( tests )
