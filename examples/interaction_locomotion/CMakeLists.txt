
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

add_executable( vsgvr_example_locomotion ${SOURCES} )
target_include_directories( vsgvr_example_locomotion PRIVATE . )
target_link_libraries( vsgvr_example_locomotion vsg::vsg vsgvr )

install(TARGETS vsgvr_example_locomotion)

set_target_properties( vsgvr_example_locomotion PROPERTIES FOLDER "examples" )
