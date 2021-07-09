
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/vsgvr/*.h )

set( SOURCES 
  src/vsgvr/VR.cpp
  src/vsgvr/VRContext.cpp
  src/vsgvr/VRDevice.cpp
  src/vsgvr/VRController.cpp

  src/vsgvr/VRViewer.cpp
  src/vsgvr/UpdateVRVisitor.cpp

  src/vsgvr/openvr/OpenVRContext.cpp
)

add_library( vsgvr STATIC ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${PROJECT_SOURCE_DIR}/vr
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC
  ${OPENVR_ROOT}/headers
)
target_link_libraries( vsgvr PUBLIC vsg::vsg ${OPENVR_LIB})
target_link_directories( vsgvr PUBLIC ${OPENVR_LIBDIR} )
