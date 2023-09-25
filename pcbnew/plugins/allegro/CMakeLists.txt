set( ALLEGRO2PCBNEW_SRCS
    allegro_file.cpp
    allegro_pcb.cpp
    allegro_plugin.cpp
    )

add_library( allegro2pcbnew STATIC ${ALLEGRO2PCBNEW_SRCS} )

target_link_libraries (allegro2pcbnew
    pcbcommon
    )

target_include_directories( allegro2pcbnew
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
