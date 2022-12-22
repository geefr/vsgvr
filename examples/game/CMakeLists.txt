
set( SOURCES 
  main.cpp
  game.h
  game.cpp
  interaction.h
  interaction.cpp

  interactions/interaction_teleport.h
  interactions/interaction_teleport.cpp
  interactions/interaction_slide.h
  interactions/interaction_slide.cpp
)

add_executable( vsgvr_example_game ${SOURCES} )
target_include_directories( vsgvr_example_game PRIVATE . )
target_link_libraries( vsgvr_example_game vsg::vsg vsgvr )

install(TARGETS vsgvr_example_game)

set_target_properties( vsgvr_example_game PROPERTIES FOLDER "examples" )
