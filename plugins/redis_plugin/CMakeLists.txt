file(GLOB HEADERS "include/eosio/redis_plugin/*.hpp")
add_library( redis_plugin
             redis_plugin.cpp
             ${HEADERS} )

target_link_libraries( redis_plugin chain_plugin appbase hiredis)
target_include_directories( redis_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
