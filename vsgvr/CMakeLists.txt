
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
    include/vsgvr/openxr/OpenXRViewer.h
    src/vsgvr/openxr/OpenXRViewer.cpp
    include/vsgvr/openxr/OpenXRContext.h
    src/vsgvr/openxr/OpenXRContext.cpp
    include/vsgvr/openxr/OpenXRVkInstance.h
    src/vsgvr/openxr/OpenXRVkInstance.cpp
    include/vsgvr/openxr/OpenXRVkPhysicalDevice.h
    src/vsgvr/openxr/OpenXRVkPhysicalDevice.cpp
    include/vsgvr/openxr/OpenXRVkDevice.h
    src/vsgvr/openxr/OpenXRVkDevice.cpp
    include/vsgvr/openxr/OpenXRWindow.h
    src/vsgvr/openxr/OpenXRWindow.cpp
    include/vsgvr/openxr/OpenXRSwapchain.h
    src/vsgvr/openxr/OpenXRSwapchain.cpp
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
