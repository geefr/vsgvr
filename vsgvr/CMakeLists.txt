
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/vsgvr/*.h )

set( SOURCES 
  src/vsgvr/VR.cpp
  src/vsgvr/VRContext.cpp
  src/vsgvr/VRDevice.cpp
  src/vsgvr/VRController.cpp

  src/vsgvr/VRViewer.cpp
  src/vsgvr/UpdateVRVisitor.cpp
)

if(VR_RUNTIME STREQUAL "openvr" )
  list(APPEND SOURCES src/vsgvr/openvr/OpenVRContext.cpp)
elseif(VR_RUNTIME STREQUAL "openxr")
  list(APPEND SOURCES src/vsgvr/openxr/openXRContext.cpp)
endif()

add_library( vsgvr STATIC ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${PROJECT_SOURCE_DIR}/vr
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries( vsgvr PUBLIC vsg::vsg )

if(VR_RUNTIME STREQUAL "openvr" )
  target_include_directories( vsgvr PUBLIC ${OPENVR_ROOT}/headers )
  target_link_libraries( vsgvr PUBLIC ${OPENVR_LIB} )
  target_link_directories( vsgvr PUBLIC ${OPENVR_LIBDIR} )
elseif(VR_RUNTIME STREQUAL "openxr")
  # target_include_directories( vsgvr PUBLIC ${PROJECT_SOURCE_DIR}/deps/ )
  target_link_libraries( vsgvr PUBLIC openxr_loader )
  # target_link_directories( vsgvr PUBLIC ${OPENVR_LIBDIR} )
endif()
