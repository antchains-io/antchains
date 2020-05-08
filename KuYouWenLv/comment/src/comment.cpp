#include <comment.hpp>
#include <kuyouwenlv.hpp>

ACTION comment::insert(name user, uint8_t type, uint64_t typeprimarykey,
                uint64_t primarykey, const std::string &content_hash) {
  require_auth(get_self());
  eosio_assert(is_account(user) == true, "username does not exist");
//   if (type == SUBJECTYPE::ROADBOOK) {
//     kuyouwenlv::reqkywl_action kywl("kuyouwenlv"_n, {_self, "active"_n});
//     kywl.send(type, typeprimarykey);
//   } else {
//     eosio_assert(false, "Not support type");
//   }

  commentsIndex.emplace(_self, [&](auto &row) {
    row.primarykey = primarykey;
    row.user = user;
    row.type = type;
    row.typeprimarykey = typeprimarykey;
    row.contenthash = content_hash;
  });
}

ACTION comment::erase(uint64_t primarykey) {
  eosio_assert(has_auth(get_self()), "Permission deny.");
  auto iterator = commentsIndex.find(primarykey);
  eosio_assert(iterator != commentsIndex.end(), "Record does not exist");
  commentsIndex.erase(iterator);
}

ACTION comment::reqcomment(uint64_t primarykey) {
  auto iterator = commentsIndex.find(primarykey);
  eosio_assert(iterator != commentsIndex.end(), "Record does not exist");
}

EOSIO_DISPATCH(comment, (insert)(erase)(reqcomment))