
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/vsgvr/*.h )

set( SOURCES 
  src/vsgvr/VR.cpp
  src/vsgvr/VRContext.cpp
  src/vsgvr/VRDevice.cpp
  src/vsgvr/VRController.cpp

  src/vsgvr/UpdateVRVisitor.cpp
)

if(VR_RUNTIME STREQUAL "openxr")
  list(APPEND SOURCES 
    include/vsgvr/openxr/OpenXR.h
    include/vsgvr/openxr/OpenXREventHandler.h
    src/vsgvr/openxr/OpenXREventHandler.cpp
    include/vsgvr/openxr/OpenXRTraits.h
    src/vsgvr/openxr/OpenXRTraits.cpp
    include/vsgvr/openxr/OpenXRInstance.h
    src/vsgvr/openxr/OpenXRInstance.cpp
    include/vsgvr/openxr/OpenXRGraphicsBindingVulkan2.h
    src/vsgvr/openxr/OpenXRGraphicsBindingVulkan2.cpp
    include/vsgvr/openxr/OpenXRSession.h
    src/vsgvr/openxr/OpenXRSession.cpp
    include/vsgvr/openxr/OpenXRVkInstance.h
    src/vsgvr/openxr/OpenXRVkInstance.cpp
    include/vsgvr/openxr/OpenXrVkPhysicalDevice.h
    src/vsgvr/openxr/OpenXrVkPhysicalDevice.cpp
    include/vsgvr/openxr/OpenXrVkDevice.h
    src/vsgvr/openxr/OpenXrVkDevice.cpp
  )
endif()

add_library( vsgvr STATIC ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${PROJECT_SOURCE_DIR}/vr
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries( vsgvr PUBLIC vsg::vsg )

if(VR_RUNTIME STREQUAL "openxr")
  target_link_libraries( vsgvr PUBLIC openxr_loader )
endif()
