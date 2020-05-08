file(GLOB HEADERS "include/eosio/kbt_plugin/*.hpp")
add_library( kbt_plugin
             kbt_plugin.cpp
             ${HEADERS} )

target_link_libraries( kbt_plugin wallet_plugin redis_plugin chain_plugin http_plugin appbase )
target_include_directories( kbt_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
