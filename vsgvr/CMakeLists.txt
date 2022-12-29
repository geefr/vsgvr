
set( HEADERS_ACTIONS
  include/vsgvr/actions/Action.h
  include/vsgvr/actions/ActionPoseBinding.h
  include/vsgvr/actions/SpaceBinding.h
  include/vsgvr/actions/ActionSet.h
)

set( HEADERS_APP
  include/vsgvr/app/Viewer.h
  include/vsgvr/app/UserOrigin.h
  include/vsgvr/app/CompositionLayers.h
)

set( HEADERS_XR
  include/vsgvr/xr/Common.h
  include/vsgvr/xr/EventHandler.h
  include/vsgvr/xr/GraphicsBindingVulkan.h
  include/vsgvr/xr/Instance.h
  include/vsgvr/xr/ProjectionMatrix.h
  include/vsgvr/xr/Session.h
  include/vsgvr/xr/Swapchain.h
  include/vsgvr/xr/Traits.h
  include/vsgvr/xr/ViewMatrix.h
)

set( SOURCES
  src/vsgvr/actions/Action.cpp
  src/vsgvr/actions/ActionPoseBinding.cpp
  src/vsgvr/actions/SpaceBinding.cpp
  src/vsgvr/actions/ActionSet.cpp
  src/vsgvr/app/Viewer.cpp
  src/vsgvr/app/UserOrigin.cpp
  src/vsgvr/app/CompositionLayers.cpp
  src/vsgvr/xr/EventHandler.cpp
  src/vsgvr/xr/GraphicsBindingVulkan.cpp
  src/vsgvr/xr/Instance.cpp
  src/vsgvr/xr/Session.cpp
  src/vsgvr/xr/Swapchain.cpp
  src/vsgvr/xr/Traits.cpp
)

if( ANDROID )
  list(APPEND HEADERS_XR
    include/vsgvr/xr/AndroidTraits.h
  )
  list(APPEND SOURCES
    src/vsgvr/xr/AndroidTraits.cpp
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
