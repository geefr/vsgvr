
if(ANDROID)
# Quest samples are loaded backwards - Open the project in Android Studio, and VSGVR will be included through CMake
else()
  add_subdirectory( generic )
endif()

