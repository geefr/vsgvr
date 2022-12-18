
add_executable( vsgvr_example_generic main.cpp )

target_link_libraries( vsgvr_example_generic vsg::vsg vsgvr )

install(TARGETS vsgvr_example_generic)

set_target_properties( vsgvr_example_generic PROPERTIES FOLDER "examples" )
