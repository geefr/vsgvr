
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/vsgvr/*.h )

set( SOURCES
  include/vsgvr/OpenXRCommon.h
  include/vsgvr/OpenXRInstance.h
  src/vsgvr/OpenXRInstance.cpp
  include/vsgvr/OpenXREventHandler.h
  src/vsgvr/OpenXREventHandler.cpp
  include/vsgvr/OpenXRTraits.h
  src/vsgvr/OpenXRTraits.cpp
  include/vsgvr/OpenXRViewer.h
  src/vsgvr/OpenXRViewer.cpp
  include/vsgvr/OpenXRGraphicsBindingVulkan2.h
  src/vsgvr/OpenXRGraphicsBindingVulkan2.cpp
  include/vsgvr/OpenXRSession.h
  src/vsgvr/OpenXRSession.cpp
  include/vsgvr/OpenXRSwapchain.h
  src/vsgvr/OpenXRSwapchain.cpp
  include/vsgvr/OpenXRVkInstance.h
  src/vsgvr/OpenXRVkInstance.cpp
  include/vsgvr/OpenXRVkPhysicalDevice.h
  src/vsgvr/OpenXRVkPhysicalDevice.cpp
  include/vsgvr/OpenXRVkDevice.h
  src/vsgvr/OpenXRVkDevice.cpp
  include/vsgvr/OpenXRViewMatrix.h
  include/vsgvr/OpenXRProjectionMatrix.h
)

add_library( vsgvr STATIC ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries( vsgvr PUBLIC vsg::vsg )
target_link_libraries( vsgvr PUBLIC openxr_loader )
