
set( SOURCES 
  main.cpp
  game.h
  game.cpp
  interaction.h
  interaction.cpp

  interactions/interaction_teleport.h
  interactions/interaction_teleport.cpp
)

add_executable( vsgvr_example_composition_layers ${SOURCES} )
target_include_directories( vsgvr_example_composition_layers PRIVATE . )
target_link_libraries( vsgvr_example_composition_layers vsg::vsg vsgvr )

install(TARGETS vsgvr_example_composition_layers)

set_target_properties( vsgvr_example_composition_layers PROPERTIES FOLDER "examples" )
