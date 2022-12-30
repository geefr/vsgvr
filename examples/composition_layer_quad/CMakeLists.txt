
set( SOURCES 
  main.cpp
  game.h
  game.cpp
  interaction.h
  interaction.cpp

  interactions/interaction_teleport.h
  interactions/interaction_teleport.cpp
)

add_executable( vsgvr_example_composition_quad ${SOURCES} )
target_include_directories( vsgvr_example_composition_quad PRIVATE . )
target_link_libraries( vsgvr_example_composition_quad vsg::vsg vsgvr )

install(TARGETS vsgvr_example_composition_quad)

set_target_properties( vsgvr_example_composition_quad PROPERTIES FOLDER "examples" )
