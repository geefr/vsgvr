cmake_minimum_required(VERSION 3.13.0)

project(vsgandroidnative)

# build native_app_glue as a static lib
set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# set the use of C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_VISIBILITY_PRESET default)

# To prevent errors when loading the native activity, ensure library name is the same in debug builds
set(CMAKE_DEBUG_POSTFIX "")

# find vsg
# find_package(vsg REQUIRED NO_CMAKE_FIND_ROOT_PATH)
# set( BUILD_SHARED_LIBS ON )
add_subdirectory(D:/dev/vsg/VulkanSceneGraph vsg)

add_subdirectory(D:/dev/vsg/vsgvr vsgvr)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set(CMAKE_SHARED_LINKER_FLAGS
    "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

# add vsgnative target
add_library(vsgnative SHARED
        main.cpp)

# add the app glue include directory
target_include_directories(vsgnative PRIVATE
    ${ANDROID_NDK}/sources/android/native_app_glue
        D:/dev/vsg/vsgvr/vsgvr/include)

# add lib dependencies
target_link_libraries(vsgnative
    vsg::vsg vsgvr
    android
    native_app_glue
    log
    )
