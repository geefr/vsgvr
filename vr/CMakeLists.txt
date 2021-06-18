
set( vr_cpp
  env.cpp
  device.cpp
  controller.cpp
)

set( vr_h
    env.h
    device.h
    controller.h
)

add_library( vrhelp STATIC ${vr_cpp} ${vr_h} )

target_include_directories(
  vrhelp PUBLIC
  ${OPENVR_ROOT}/headers
  ${Vulkan_INCLUDE_DIRS}
)

target_compile_definitions( vrhelp PUBLIC VR_SUBMIT_VULKAN )
target_link_libraries( vrhelp PUBLIC ${OPENVR_LIB} )
target_link_directories( vrhelp PUBLIC ${OPENVR_LIBDIR} )
if( MSVC )
  target_link_libraries( vrhelp PUBLIC glm )
endif()
