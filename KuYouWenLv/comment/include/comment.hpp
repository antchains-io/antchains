
#include <eosiolib/eosio.hpp>
using namespace eosio;

CONTRACT comment : public eosio::contract {
public:
  using contract::contract;
  enum SUBJECTYPE { COMMENT, ROADBOOK, AIRTICKETS, HOTEL };

  comment(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds), commentsIndex(_self, _self.value) {}

  ACTION insert(name user, uint8_t type, uint64_t typeprimarykey,
                uint64_t primarykey, const std::string &content_hash);
  ACTION erase(uint64_t primarykey);
  ACTION reqcomment(uint64_t primarykey);

  using insert_action = action_wrapper<"insert"_n, &comment::insert>;
  using erase_action = action_wrapper<"erase"_n, &comment::erase>;
  using reqcomment_action = action_wrapper<"reqcomment"_n, &comment::reqcomment>;

private:
  TABLE usercomments {
    name user;
    uint64_t type;
    uint64_t typeprimarykey;
    uint64_t primarykey;
    std::string contenthash;
    uint64_t primary_key() const { return primarykey; }
    uint64_t get_user_name() const { return user.value; }
    uint64_t get_type() const { return type; }
    uint64_t get_typeprimarykey() const { return typeprimarykey; }
  };

  typedef eosio::multi_index<
      "comments"_n, usercomments,
      indexed_by<"byuser"_n, const_mem_fun<usercomments, uint64_t,
                                           &usercomments::get_user_name>>,
      indexed_by<"bytype"_n, const_mem_fun<usercomments, uint64_t,
                                           &usercomments::get_type>>,
      indexed_by<"bytypepkey"_n,
                 const_mem_fun<usercomments, uint64_t,
                               &usercomments::get_typeprimarykey>>>
      comments_index;

  comments_index commentsIndex;
};