#include <kuyouwenlv.hpp>

ACTION kuyouwenlv::insert(name user, uint8_t type, uint64_t primarykey,
                          uint64_t order_amount) {
   require_auth(user);
   eosio_assert(is_account(user) == true, "username does not exist");

   auto iterator = kuyouwenlvIndex.find(primarykey);
   eosio_assert(iterator == kuyouwenlvIndex.end(), "this order exist.");
   eosio_assert(type == 0, "only user can create order and type is 0.");
   kuyouwenlvIndex.emplace(_self, [&](auto &row) {
      row.user = user;
      row.type = type;
      row.primarykey = primarykey;
      row.order_amount = order_amount;
   });
}

ACTION kuyouwenlv::updatetype(name user, uint8_t type, uint64_t primarykey,
                          uint64_t order_amount) {
   require_auth(_self);
   eosio_assert(is_account(user) == true, "username does not exist");

   auto iterator = kuyouwenlvIndex.find(primarykey);
   eosio_assert(iterator != kuyouwenlvIndex.end(), "this order not exist.");
   eosio_assert(type > 0 && type <= 2, "only admin's permission can modify order type 1 or 2.");
   kuyouwenlvIndex.modify(iterator, _self, [&](auto &row) {
      row.type = type;
   });
}

ACTION kuyouwenlv::erase(uint64_t primarykey) {
  require_auth(get_self());
  auto iterator = kuyouwenlvIndex.find(primarykey);
  eosio_assert(iterator != kuyouwenlvIndex.end(),
               "Record does not exist");
  kuyouwenlvIndex.erase(iterator);
}

ACTION kuyouwenlv::reqkywl(uint64_t primarykey) {
  auto iterator = kuyouwenlvIndex.find(primarykey);
  eosio_assert(iterator != kuyouwenlvIndex.end(), "Record does not exist");
}

EOSIO_DISPATCH(kuyouwenlv, (insert)(updatetype)(erase)(reqkywl))
