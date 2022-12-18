
set( HEADERS_ACTIONS
  include/vsgvr/actions/OpenXRAction.h
  include/vsgvr/actions/OpenXRActionPoseBinding.h
  include/vsgvr/actions/OpenXRActionSet.h
)

set( HEADERS_APP
  include/vsgvr/app/OpenXRViewer.h
)

set( HEADERS_XR
  include/vsgvr/xr/OpenXRCommon.h
  include/vsgvr/xr/OpenXREventHandler.h
  include/vsgvr/xr/OpenXRGraphicsBindingVulkan.h
  include/vsgvr/xr/OpenXRInstance.h
  include/vsgvr/xr/OpenXRProjectionMatrix.h
  include/vsgvr/xr/OpenXRSession.h
  include/vsgvr/xr/OpenXRSwapchain.h
  include/vsgvr/xr/OpenXRTraits.h
  include/vsgvr/xr/OpenXRViewMatrix.h
)

set( SOURCES
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
  list(APPEND HEADERS_XR
    include/vsgvr/xr/OpenXRAndroidTraits.h
  )
  list(APPEND SOURCES
    src/vsgvr/xr/OpenXRAndroidTraits.cpp
  )
endif()

add_library( vsgvr ${HEADERS_ACTIONS} ${HEADERS_APP} ${HEADERS_XR} ${SOURCES} )

target_include_directories(vsgvr PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries( vsgvr PUBLIC vsg::vsg ${OpenXR_LIBRARY} )

set_target_properties( vsgvr PROPERTIES FOLDER "vsgvr" )

# TODO: This needs to be in a header - Otherwise anyone linking to vsgvr without
#       going through CMake will have to define it themselves
if(WIN32 AND BUILD_SHARED_LIBS)
  target_compile_definitions(vsgvr PRIVATE "VSGVR_DECLSPEC=__declspec(dllexport)")
  target_compile_definitions(vsgvr INTERFACE "VSGVR_DECLSPEC=__declspec(dllimport)")
else()
  target_compile_definitions(vsgvr PUBLIC "VSGVR_DECLSPEC=")
endif()

set_target_properties(vsgvr PROPERTIES
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    SOVERSION ${VSGVR_SOVERSION}
)

install(TARGETS vsgvr ${INSTALL_TARGETS_DEFAULT_FLAGS})
install(FILES ${HEADERS_ACTIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vsgvr/actions)
install(FILES ${HEADERS_APP} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vsgvr/app)
install(FILES ${HEADERS_XR} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vsgvr/xr)
