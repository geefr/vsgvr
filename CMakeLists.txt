cmake_minimum_required(VERSION 3.14)

project(vsgvr
    VERSION 0.4.0
    DESCRIPTION "VulkanSceneGraph based virtual reality viewer"
    LANGUAGES CXX
)

include(FetchContent)

# Compiler/Tool requirements
set( CMAKE_CXX_STANDARD 17 )
set(OpenGL_GL_PREFERENCE GLVND)

# Package/System requirements
find_package(vsg 1.0.0 NO_CMAKE_FIND_ROOT_PATH)
find_package(Vulkan REQUIRED)

vsg_setup_dir_vars()
vsg_setup_build_vars()
vsg_add_target_clobber()
vsg_add_target_uninstall()

add_subdirectory( deps/openxr )

add_subdirectory( vsgvr )

# setup relative data install path
file(RELATIVE_PATH VSGVR_REL_DATADIR ${CMAKE_INSTALL_FULL_BINDIR} ${CMAKE_INSTALL_FULL_DATADIR}/vsgvr)
add_definitions(-DVSGVR_REL_DATADIR=\"${VSGVR_REL_DATADIR}\")

add_executable( example_vr example_vr.cpp )
target_link_libraries( example_vr vsg::vsg vsgvr )
target_include_directories( example_vr PRIVATE 
  ${OPENVR_ROOT}/headers
  vsgvr/include )

install(TARGETS example_vr)

# Copy models into the build dir
configure_file(models/world/world.vsgt ${CMAKE_CURRENT_BINARY_DIR}/share/vsgvr/world.vsgt COPYONLY)
configure_file(models/controller/controller.vsgt ${CMAKE_CURRENT_BINARY_DIR}/share/vsgvr/controller.vsgt COPYONLY)
configure_file(models/controller/controller2.vsgt ${CMAKE_CURRENT_BINARY_DIR}/share/vsgvr/controller2.vsgt COPYONLY)

install(FILES
    models/world/world.vsgt
    models/controller/controller.vsgt
    models/controller/controller2.vsgt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vsgvr
)
