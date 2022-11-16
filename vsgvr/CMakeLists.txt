
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/vsgvr/*.h )

set( SOURCES
  include/vsgvr/xr/OpenXRCommon.h
  include/vsgvr/xr/OpenXRInstance.h
  src/vsgvr/xr/OpenXRInstance.cpp
  include/vsgvr/xr/OpenXREventHandler.h
  src/vsgvr/xr/OpenXREventHandler.cpp
  include/vsgvr/xr/OpenXRTraits.h
  src/vsgvr/xr/OpenXRTraits.cpp
  include/vsgvr/app/OpenXRViewer.h
  src/vsgvr/app/OpenXRViewer.cpp
  include/vsgvr/xr/OpenXRGraphicsBindingVulkan.h
  src/vsgvr/xr/OpenXRGraphicsBindingVulkan.cpp
  include/vsgvr/xr/OpenXRSession.h
  src/vsgvr/xr/OpenXRSession.cpp
  include/vsgvr/xr/OpenXRSwapchain.h
  src/vsgvr/xr/OpenXRSwapchain.cpp
  include/vsgvr/xr/OpenXRViewMatrix.h
  include/vsgvr/xr/OpenXRProjectionMatrix.h

  include/vsgvr/actions/OpenXRAction.h
  src/vsgvr/actions/OpenXRAction.cpp
  include/vsgvr/actions/OpenXRActionSet.h
  src/vsgvr/actions/OpenXRActionSet.cpp
  include/vsgvr/actions/OpenXRActionPoseBinding.h
  src/vsgvr/actions/OpenXRActionPoseBinding.cpp
)

add_library( vsgvr ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries( vsgvr PUBLIC vsg::vsg )
target_link_libraries( vsgvr PUBLIC openxr_loader )
if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_options(vsgvr PRIVATE "-DVSGVR_DECLSPEC=__declspec(dllexport)")
    target_compile_options(vsgvr INTERFACE "-DVSGVR_DECLSPEC=__declspec(dllimport)")
else()
    target_compile_options(vsgvr PUBLIC -DVSGVR_DECLSPEC=)
endif()

install(TARGETS vsgvr ${INSTALL_TARGETS_DEFAULT_FLAGS})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vsgvr)

vsg_add_cmake_support_files(
    CONFIG_TEMPLATE vsgvrConfig.cmake.in
)
