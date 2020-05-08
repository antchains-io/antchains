
#include <thumbup.hpp>
#include <kuyouwenlv.hpp>
#include <comment.hpp>
using namespace eosio;

ACTION thumbup::insert(name user, uint8_t type, uint64_t typeprimarykey,
              uint64_t primarykey) {
  require_auth(_self);
  eosio_assert(is_account(user) == true, "username does not exist");
  thumbups_index thumbupsIndex(_self, _self.value);

//   if (type == SUBJECTYPE::COMMENT) {
//     comment::reqcomment_action reqcomment("comment"_n, {_self, "active"_n});
//     reqcomment.send(typeprimarykey);
//   } else if (type == SUBJECTYPE::ROADBOOK) {
//     kuyouwenlv::reqkywl_action kywl("kuyouwenlv"_n, {_self, "active"_n});
//     kywl.send(type, typeprimarykey);
//   } else {
//     eosio_assert(false, "Not support type");
//   }

  auto iterator = thumbupsIndex.find(primarykey);
  eosio_assert(iterator == thumbupsIndex.end(), "Record exist");
  thumbupsIndex.emplace(_self, [&](auto &row) {
    row.primarykey = primarykey;
    row.user = user;
    row.type = type;
    row.typeprimarykey = typeprimarykey;
  });
}

ACTION thumbup::erase(uint64_t primarykey) {
  require_auth(_self);
  auto iterator = thumbupsIndex.find(primarykey);
  eosio_assert(iterator != thumbupsIndex.end(), "Record does not exist");
  thumbupsIndex.erase(iterator);
}

EOSIO_DISPATCH(thumbup, (insert)(erase))