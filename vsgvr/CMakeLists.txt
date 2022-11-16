
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

add_library( vsgvr ${SOURCES} ${HEADERS} )

target_include_directories(
  vsgvr PRIVATE
  ${PROJECT_SOURCE_DIR}/vr
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PUBLIC
  ${OPENVR_INCLUDE_DIRS}
)
target_link_libraries( vsgvr PUBLIC vsg::vsg ${OPENVR_LIB})
target_link_directories( vsgvr PUBLIC ${OPENVR_LIBDIR} )
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
        OUTPUT_NAME vsgvr-${VSGVR_SOVERSION}
    )
endif()

install(TARGETS vsgvr ${INSTALL_TARGETS_DEFAULT_FLAGS})
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vsgvr)

vsg_add_cmake_support_files(
    CONFIG_TEMPLATE vsgvrConfig.cmake.in
)
