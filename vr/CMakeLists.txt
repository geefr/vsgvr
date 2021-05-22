
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

include_directories( ${OPENVR_ROOT}/headers )
link_directories( ${OPENVR_ROOT}/lib/linux64 )

add_library( vrhelp STATIC ${vr_cpp} ${vr_h} )
target_compile_definitions( vrhelp PUBLIC VR_SUBMIT_VULKAN )

target_link_libraries( vrhelp ${OPENVR_LIB} )
if( MSVC )
  target_link_libraries( vrhelp glm )
endif()

