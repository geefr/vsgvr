# VSGVR Example for Android - Monado runtime

This example demonstrates basic VSG and VSGVR usage on any Android device, running the Monado OpenXR runtime.

Build Requirements:
* Android Studio
* Android SDK API Level 27
* Android NDK 25
* Android device with Monado OpenXR runtime installed

Quirks / Issues:
* If ANDROID_STL=C++_static the OpenXR runtime will not load correctly. The runtime could be rebuilt with a static STL, but the simpler option is to use c++_shared in the vsgvr application
* Monado on Android seems to be targeted at cardboard-like HMDs - Configuring as XR_FORM_FACTOR_HANDHELD_DISPLAY is not currently possible
* When starting the application, ensure phone is held horizontally - Tracking is relative to the starting position
* Performance may be quite low

## Installing Monado

Firstly, build and install the monado runtime from https://gitlab.freedesktop.org/monado/monado
* Clone the repo
* Open the repo in Android Studio
* Build and install the runtime APK

Also build and install the monado broker application from https://gitlab.freedesktop.org/monado/utilities/openxr-android-broker
* Clone the repo
* Open the repo in Android Studio
* Build and install the APK for `installable_runtime_broker`

On the phone, configure the runtime
* Open the app `OpenXR Runtime Broker`
* Ensure the Monado runtime is selected

## Build Configuration

The Android Studio project assumes the following folder structure:
* VulkanSceneGraph
* vsgvr
  * examples
    * android-monado

Project / Gradle requirements
* VSGVR_XR_PLATFORM=OPENXR_GENERIC
* ANDROID_STL=c++_shared
* `implementation 'org.khronos.openxr:openxr_loader_for_android:1.0.23'`

If this structure is modified, or your vsg is located in another location, modify app/build.gradle and app/cpp/CMakeLists.txt accordingly, to specify different locations for VSG and the Oculus OpenXR SDK.

For simplicity this sample includes VSG directly into the CMake project, but with modifications may load from a prebuilt copy as well.

## Building

* Open project in Android Studio
* Build / Run
