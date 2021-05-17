
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

include_directories( ${OPENVR_ROOT}/include )
link_directories( ${OPENVR_ROOT}/lib )

add_library( vrhelp STATIC ${vr_cpp} ${vr_h} )

target_link_libraries( vrhelp ${OPENVR_LIB} )
if( MSVC )
  target_link_libraries( vrhelp glm )
endif()

