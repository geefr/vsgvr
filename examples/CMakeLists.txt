
if(ANDROID)
# Quest samples are loaded backwards - Open the project in Android Studio, and VSGVR will be included through CMake
# - minimal_quest
# - locomotion/quest
else()
  add_subdirectory( generic )
  add_subdirectory( interaction_locomotion )
  add_subdirectory( composition_layer_quad )
endif()

