file(GLOB HEADERS "include/eosio/evs_plugin/*.hpp")
add_library( evs_plugin
             evs_plugin.cpp
             ${HEADERS} )

target_link_libraries( evs_plugin chain_plugin http_plugin appbase )
target_include_directories( evs_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
