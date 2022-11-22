
file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/vsgvr/*/*.h )

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

add_library( vsgvr ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries( vsgvr PUBLIC vsg::vsg )
target_link_libraries( vsgvr PUBLIC ${OpenXR_LIBRARY})
if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_options(vsgvr PRIVATE "-DVSGVR_DECLSPEC=__declspec(dllexport)")
    target_compile_options(vsgvr INTERFACE "-DVSGVR_DECLSPEC=__declspec(dllimport)")
else()
    target_compile_options(vsgvr PUBLIC -DVSGVR_DECLSPEC=)
endif()
set_target_properties(vsgvr PROPERTIES
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    SOVERSION ${VSGVR_SOVERSION}
)
if(WIN32)
    set_target_properties(vsgvr PROPERTIES
        RUNTIME_OUTPUT_NAME vsgvr-${VSGVR_SOVERSION}
    )
endif()

install(TARGETS vsgvr ${INSTALL_TARGETS_DEFAULT_FLAGS})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vsgvr)

vsg_add_cmake_support_files(
    CONFIG_TEMPLATE vsgvrConfig.cmake.in
)
