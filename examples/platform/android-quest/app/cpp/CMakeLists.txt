cmake_minimum_required(VERSION 3.13.0)

project(vsgvrquestnative)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# set the use of C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_VISIBILITY_PRESET default)

# To prevent errors when loading the native activity, ensure library name is the same in all builds
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "")

# find vsg
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../../../VulkanSceneGraph" vsg)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../../" vsgvr)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# add vsgnative target
add_library(vsgvrquestnative SHARED main.cpp)

# add the app glue include directory
target_include_directories(vsgvrquestnative PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vsgvr/include
    )

# add lib dependencies
target_link_libraries(vsgvrquestnative
    vsg::vsg vsgvr
    android
    native_app_glue
    log
    )
