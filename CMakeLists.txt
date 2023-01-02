cmake_minimum_required(VERSION 3.14)

project(vsgvr
    VERSION 0.4.0
    DESCRIPTION "VulkanSceneGraph based virtual reality viewer"
    LANGUAGES CXX
)
set(VSGVR_SOVERSION 0)


option( BUILD_SHARED_LIBS "Build shared libraries" OFF )
option( VSGVR_BUILD_EXAMPLES "Whether to build VSGVR examples" ON )

if( NOT VSGVR_XR_PLATFORM )
  # OPENXR_GENERIC - The standard openxr_loader.so, built from deps/openxr
  # OPENXR_SYSTEM - Whatever OpenXR is available via find_package
  # OPENXR_OCULUS_MODULE - Oculus specific runtime from their Mobile OpenXR SDK
  set( VSGVR_XR_PLATFORM "OPENXR_GENERIC" CACHE STRING "The OpenXR loader / platform combination to build" )
  set_property( CACHE VSGVR_XR_PLATFORM PROPERTY STRINGS "OPENXR_GENERIC" "OPENXR_SYSTEM" "OPENXR_OCULUS_MOBILE" )
endif()

# Compiler/Tool requirements
set( CMAKE_CXX_STANDARD 17 )
set( OpenGL_GL_PREFERENCE GLVND )

# Package/System requirements
if(ANDROID)
  # On Android, allow vsg to be located outside the Android sysroot
  find_package(vsg 1.0.0 NO_CMAKE_FIND_ROOT_PATH)
else()
  find_package(vsg 1.0.0)
endif()

# Other Dependencies
find_package(Vulkan REQUIRED)

set(OpenXR_LIBRARY openxr_loader)
if( VSGVR_XR_PLATFORM STREQUAL "OPENXR_GENERIC" )
  add_subdirectory( deps/openxr )
elseif( VSGVR_XR_PLATFORM STREQUAL "OPENXR_SYSTEM" )
  find_package(OpenXR REQUIRED)
  set(OpenXR_LIBRARY OpenXR::openxr_loader)
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


# vsgMacros.cmake - Follow vsg conventions for the most part,
# for all vsgvr libraries and executables
vsg_setup_dir_vars()
vsg_setup_build_vars()
set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "MinSizeRel" "RelWithDebInfo" )
get_property( generator_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )
if( generator_is_multi_config )
  set( CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo" CACHE STRING "Build configuration types")
endif()

# TODO: Will re-visit these, and fix when vsgvr is more mature
# (Dev primarily on Windows/Visual Studio, or in Android Studio)
#
# vsg_add_target_clang_format(
#     FILES
#         ${PROJECT_SOURCE_DIR}/vsgvr/include/vsgvr/*.h
#         ${PROJECT_SOURCE_DIR}/vsgvr/srv/vsgvr/*.cpp
# )
# vsg_add_target_cppcheck(
#     FILES
#         ${PROJECT_SOURCE_DIR}/vsgvr/include/vsgvr/*.h
#         ${PROJECT_SOURCE_DIR}/vsgvr/srv/vsgvr/*.cpp
# )
# vsg_add_target_docs(
#     FILES
#         ${PROJECT_SOURCE_DIR}/vsgvr/include
# )
# vsg_add_target_clobber()

vsg_add_target_uninstall()
set_target_properties(uninstall-vsgvr PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)

vsg_add_feature_summary()
vsg_add_cmake_support_files(
  CONFIG_TEMPLATE vsgvrConfig.cmake.in
)

# VSGVR
add_subdirectory( vsgvr )

if( VSGVR_BUILD_EXAMPLES )
  add_subdirectory( examples )
endif()

