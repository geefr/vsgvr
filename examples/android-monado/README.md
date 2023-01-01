# VSGVR Example for Android - Monado runtime

This example demonstrates basic VSG and VSGVR usage on any Android device, running the Monado OpenXR runtime.

Build Requirements:
* Android Studio
* Android SDK API Level 24
* Android NDK 25
* TODO: Monado setup details

## Build Configuration

The Android Studio project assumes the following folder structure:
* VulkanSceneGraph
* vsgvr
  * examples
    * android-monado

If this structure is modified, or your vsg is located in another location, modify app/build.gradle and app/cpp/CMakeLists.txt accordingly, to specify different locations for VSG and the Oculus OpenXR SDK.

For simplicity this sample includes VSG directly into the CMake project, but with modifications may load from a prebuilt copy as well.


## Building with Android Studio

    1. Open Android Studio and select 'Open Project'
    2. Select the `questnative` folder
    3. Build the application

