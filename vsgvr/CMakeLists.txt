
set( SOURCES
  include/vsgvr/actions/OpenXRAction.h
  include/vsgvr/actions/OpenXRActionPoseBinding.h
  include/vsgvr/actions/OpenXRActionSet.h
  include/vsgvr/app/OpenXRViewer.h
  include/vsgvr/xr/OpenXRCommon.h
  include/vsgvr/xr/OpenXREventHandler.h
  include/vsgvr/xr/OpenXRGraphicsBindingVulkan.h
  include/vsgvr/xr/OpenXRInstance.h
  include/vsgvr/xr/OpenXRProjectionMatrix.h
  include/vsgvr/xr/OpenXRSession.h
  include/vsgvr/xr/OpenXRSwapchain.h
  include/vsgvr/xr/OpenXRTraits.h
  include/vsgvr/xr/OpenXRViewMatrix.h

  src/vsgvr/actions/OpenXRAction.cpp
  src/vsgvr/actions/OpenXRActionPoseBinding.cpp
  src/vsgvr/actions/OpenXRActionSet.cpp
  src/vsgvr/app/OpenXRViewer.cpp
  src/vsgvr/xr/OpenXREventHandler.cpp
  src/vsgvr/xr/OpenXRGraphicsBindingVulkan.cpp
  src/vsgvr/xr/OpenXRInstance.cpp
  src/vsgvr/xr/OpenXRSession.cpp
  src/vsgvr/xr/OpenXRSwapchain.cpp
  src/vsgvr/xr/OpenXRTraits.cpp
)

if( ANDROID )
  list(APPEND SOURCES
    include/vsgvr/xr/OpenXRAndroidTraits.h
    src/vsgvr/xr/OpenXRAndroidTraits.cpp
  )
endif()

add_library( vsgvr ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries( vsgvr PUBLIC vsg::vsg ${OpenXR_LIBRARY} )

if(WIN32 AND BUILD_SHARED_LIBS)
  target_compile_definitions(vsgvr PRIVATE VSGVR_DECLSPEC="__declspec(dllexport)")
  target_compile_definitions(vsgvr INTERFACE VSGVR_DECLSPEC="__declspec(dllimport)")
else()
  target_compile_definitions( vsgvr PUBLIC VSGVR_DECLSPEC= )
endif()

