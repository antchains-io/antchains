#include <switchtrans.hpp>

ACTION switchtrans::settrans(uint8_t switch_state)
{
   require_auth(get_self());
   eosio_assert( switch_state < 2 , "switch_state 0 is closed, 1 is open." );
   _userstate.switch_state = switch_state;
}
EOSIO_DISPATCH( switchtrans, (settrans))
