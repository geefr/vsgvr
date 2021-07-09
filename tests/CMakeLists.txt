

add_executable( test_openvr_extensions test_openvr_extensions.cpp )
target_link_libraries( test_openvr_extensions vsg::vsg vsgvr )
target_include_directories( test_openvr_extensions PRIVATE 
  ${OPENVR_ROOT}/headers 
  ${PROJECT_SOURCE_DIR}/vsgvr/include
)
