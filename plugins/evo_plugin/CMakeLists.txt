file(GLOB HEADERS "include/eosio/evo_plugin/*.hpp")
add_library( evo_plugin
             evo_plugin.cpp
             ${HEADERS} )

target_link_libraries( evo_plugin chain_plugin kbt_plugin appbase )
target_include_directories( evo_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
