
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/vsgvr/*.h )

set( SOURCES 
  src/vsgvr/VR.cpp
  src/vsgvr/UpdateVRVisitor.cpp
  src/vsgvr/VRViewer.cpp
)

add_library( vsgvr STATIC ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${PROJECT_SOURCE_DIR}/vr
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries( vsgvr PUBLIC vsg::vsg vrhelp )
