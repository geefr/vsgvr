
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
    src/vsgvr/openxr/OpenXRContext.cpp)
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
