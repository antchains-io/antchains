
#include <eosiolib/eosio.hpp>
using namespace eosio;

CONTRACT thumbup : public eosio::contract {

public:
  using contract::contract;
  enum SUBJECTYPE { COMMENT, ROADBOOK, AIRTICKETS, HOTEL };

  thumbup(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds), thumbupsIndex(_self, _self.value) {}

  ACTION insert(name user, uint8_t type, uint64_t typeprimarykey,
                uint64_t primarykey);
  ACTION erase(uint64_t primarykey);

private:
  TABLE userthumbups {
    uint64_t primarykey;
    name user;
    uint64_t type;
    uint64_t typeprimarykey;
    uint64_t primary_key() const { return primarykey; }
    uint64_t get_user_name() const { return user.value; }
    uint64_t get_type() const { return type; }
    uint64_t get_typeprimarykey() const { return typeprimarykey; }
  };

  using insert_action = action_wrapper<"insert"_n, &thumbup::insert>;
  using erase_action = action_wrapper<"erase"_n, &thumbup::erase>;

  typedef eosio::multi_index<
      "thumbups"_n, userthumbups,
      indexed_by<"byuser"_n, const_mem_fun<userthumbups, uint64_t,
                                           &userthumbups::get_user_name>>,
      indexed_by<"bytype"_n, const_mem_fun<userthumbups, uint64_t,
                                           &userthumbups::get_type>>,
      indexed_by<"bytypepkey"_n,
                 const_mem_fun<userthumbups, uint64_t,
                               &userthumbups::get_typeprimarykey>>>
      thumbups_index;

  thumbups_index thumbupsIndex;
};