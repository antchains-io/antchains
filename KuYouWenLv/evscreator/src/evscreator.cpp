#include <evscreator.hpp>

ACTION evscreator::updatecheck(name user, uint8_t from, uint8_t to) {
   if (from == 0 && to == 1) {
      require_auth(_self);
   }
   else if (from == 0 && to == 2){
      require_auth(user);
   }
   else if (from == 2 && to == 1){
      require_auth(_self);
   }
}

ACTION evscreator::updatevalue(name user, uint64_t to) {
   require_auth(_self);
}

EOSIO_DISPATCH(evscreator, (updatecheck)(updatevalue))
