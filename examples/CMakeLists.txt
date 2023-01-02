
if(ANDROID)
# Android applications are loaded backwards
# The android studio project is opened, which will include
# VSGVR, VSG, and other dependencies from parent directories
# See Android example README.md files for the required directory
# layouts and setup instructions.
else()
  add_subdirectory( platform/generic )
  add_subdirectory( features/interaction_locomotion )
  add_subdirectory( features/composition_layer_quad )
endif()

