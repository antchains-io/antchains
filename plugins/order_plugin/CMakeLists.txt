file(GLOB HEADERS "include/eosio/order_plugin/*.hpp")
add_library( order_plugin
             order_plugin.cpp
             ${HEADERS} )

target_link_libraries( order_plugin chain_plugin kbt_plugin appbase )
target_include_directories( order_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
