cmake_minimum_required(VERSION 3.14)

project(vsgvr
    VERSION 0.4.0
    DESCRIPTION "VulkanSceneGraph based virtual reality viewer"
    LANGUAGES CXX
)

set(VSGVR_SOVERSION 0)

include(FetchContent)

# Compiler/Tool requirements
set( CMAKE_CXX_STANDARD 17 )
set(OpenGL_GL_PREFERENCE GLVND)

# set( FORMAT_SKIP_CMAKE ON )
# add_subdirectory(deps/Format.cmake)

# Package/System requirements
find_package(vsg 1.0.0 REQUIRED)
find_package(Vulkan REQUIRED)

vsg_setup_dir_vars()
vsg_setup_build_vars()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
    pkg_search_module(OPENVR openvr)
endif()
if (OPENVR_FOUND)
    set(OPENVR_LIB ${OPENVR_LIBRARIES})
    set(OPENVR_LIBDIR ${OPENVR_LIBRARY_DIRS})
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(openvr
        FOUND_VAR OPENVR_FOUND
        REQUIRED_VARS
            OPENVR_LIBRARIES
            OPENVR_INCLUDE_DIRS
        VERSION_VAR OPENVR_VERSION
    )
elseif(IS_DIRECTORY deps/openvr)
    message(STATUS "Using embedded openvr development kit")
    set(OPENVR_LIB "openvr_api" )
    set(OPENVR_ROOT ${PROJECT_SOURCE_DIR}/deps/openvr)
    set(OPENVR_INCLUDE_DIRS ${OPENVR_ROOT}/headers)
    if( MSVC )
        set(OPENVR_BINDIR ${OPENVR_ROOT}/bin/win64)
        set(OPENVR_LIBDIR ${OPENVR_ROOT}/lib/win64)
    else()
        set(OPENVR_BINDIR ${OPENVR_ROOT}/bin/linux64)
        set(OPENVR_LIBDIR ${OPENVR_ROOT}/lib/linux64)
    endif()
    add_subdirectory(deps/openvr)
else()
    message(FATAL_ERROR "Could not find openvr development kit")
endif()

vsg_add_target_clang_format(
    FILES
        ${PROJECT_SOURCE_DIR}/vsgvr/include/vsgvr/*.h
        ${PROJECT_SOURCE_DIR}/vsgvr/srv/vsgvr/*.cpp
)
vsg_add_target_cppcheck(
    FILES
        ${PROJECT_SOURCE_DIR}/vsgvr/include/vsgvr/*.h
        ${PROJECT_SOURCE_DIR}/vsgvr/srv/vsgvr/*.cpp
)
vsg_add_target_clobber()
vsg_add_target_docs(
    FILES
        ${PROJECT_SOURCE_DIR}/vsgvr/include
)
vsg_add_target_uninstall()

#####################

add_subdirectory( vsgvr )

# setup relative data install path
file(RELATIVE_PATH VSGVR_REL_DATADIR ${CMAKE_INSTALL_FULL_BINDIR} ${CMAKE_INSTALL_FULL_DATADIR}/vsgvr)
add_definitions(-DVSGVR_REL_DATADIR=\"${VSGVR_REL_DATADIR}\")

add_executable( example_vr example_vr.cpp )
target_link_libraries( example_vr vsg::vsg vsgvr )
target_include_directories( example_vr PRIVATE 
  ${OPENVR_INCLUDE_DIRS}
  vsgvr/include )
if( MSVC )
  set_target_properties(example_vr PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${OPENVR_BINDIR}")
endif()

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

add_subdirectory( tests )

vsg_add_feature_summary()

