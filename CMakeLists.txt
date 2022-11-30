cmake_minimum_required(VERSION 3.14)

project(VSGVR)

option( BUILD_SHARED_LIBS "Build shared libraries" OFF )
option( VSGVR_BUILD_EXAMPLES "Whether to build VSGVR examples or not" ON )

if( NOT CMAKE_BUILD_TYPE AND NOT MSVC )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
  set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "MinSizeRel" "RelWithDebInfo" )
endif()

if( NOT VSGVR_XR_PLATFORM )
  set( VSGVR_XR_PLATFORM "OPENXR_GENERIC" CACHE STRING "The OpenXR loader / platform combination to build" )
  set_property( CACHE VSGVR_XR_PLATFORM PROPERTY STRINGS "OPENXR_GENERIC" "OPENXR_OCULUS_MOBILE" )
endif()

# Compiler/Tool requirements
set( CMAKE_CXX_STANDARD 17 )
set( OpenGL_GL_PREFERENCE GLVND )

# Package/System requirements
if(ANDROID)
  find_package(vsg 1.0.0 NO_CMAKE_FIND_ROOT_PATH)
else()
  find_package(vsg 1.0.0)
endif()

find_package(Vulkan REQUIRED)

if( VSGVR_XR_PLATFORM STREQUAL "OPENXR_GENERIC" )
  add_subdirectory( deps/openxr )
elseif( VSGVR_XR_PLATFORM STREQUAL "OPENXR_OCULUS_MOBILE" )
  if(NOT ANDROID)
    message(FATAL_ERROR "Oculus Mobile SDK is only supported when building for Android")
  endif()
  set( VSGVR_OCULUS_OPENXR_MOBILE_SDK "${CMAKE_CURRENT_SOURCE_DIR}/../ovr_openxr_mobile_sdk")
  if(NOT EXISTS ${VSGVR_OCULUS_OPENXR_MOBILE_SDK})
    message(FATAL_ERROR "Failed to find Oculus Mobile SDK, please download from https://developer.oculus.com/downloads/package/oculus-openxr-mobile-sdk/ and set VSGVR_OCULUS_OPENXR_MOBILE_SDK")
  endif()

  add_library(openxr_loader SHARED IMPORTED)
  set_property(TARGET openxr_loader PROPERTY IMPORTED_LOCATION
    "${VSGVR_OCULUS_OPENXR_MOBILE_SDK}/OpenXR/Libs/Android/${ANDROID_ABI}/${CMAKE_BUILD_TYPE}/libopenxr_loader.so")
  target_include_directories(openxr_loader INTERFACE 
    "${VSGVR_OCULUS_OPENXR_MOBILE_SDK}/OpenXR/Include"
    "${VSGVR_OCULUS_OPENXR_MOBILE_SDK}/3rdParty/khronos/openxr/OpenXR-SDK/include")
endif()

add_subdirectory( vsgvr )

if( VSGVR_BUILD_EXAMPLES )
  add_subdirectory( examples )
endif()

