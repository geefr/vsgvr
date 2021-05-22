

add_executable( test_openvr_extensions test_openvr_extensions.cpp )
target_link_libraries( test_openvr_extensions vrhelp fmt::fmt vsg::vsg)
target_link_directories( test_openvr_extensions PRIVATE ${OPENVR_ROOT}/lib/linux64 )
target_include_directories( test_openvr_extensions PRIVATE 
  ${OPENVR_ROOT}/headers 
  ${PROJECT_SOURCE_DIR}
)
