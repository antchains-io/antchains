#include<eosiolib/action.h>
#include <eosiolib/action.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <math.h>

using namespace eosio;
#define PRIVILEGE_ENTRY_ACCOUNT  eosio::name("entryacc")
#define ENTRY_ID_PRIOR_INITIAL   (uint64_t)0x0
#define ENTRY_PERMISSION_INITIAL (uint64_t)0x0
#define ENTRY_PERMISSION_EFFECTIVE (uint64_t)0x1

CONTRACT entrydetail : public eosio::contract {

public:
   using contract::contract;
   enum SUBJECTYPE { COMMENT, ROADBOOK, AIRTICKETS, HOTEL, OTHER };

   entrydetail(name receiver, name code, datastream<const char *> ds)
         : contract(receiver, code, ds),
         entryIndex(receiver, receiver.value) {}

   ACTION entrycreate(name user, uint64_t entry_id,uint64_t entry_id_prior,std::vector<permission_level>  permisson,uint64_t type);
   ACTION entryeffect(uint64_t entry_id,permission_level permission,asset cash_back);
   ACTION entrycancle(uint64_t entry_id);
   bool entry_type_is_valid();
   //ACTION entry_query(uint64_t entry_id);

private:
  TABLE entryinfor {
    name user;
    uint64_t entry_id;
    uint64_t entry_id_prior;
    std::vector<std::pair<permission_level,uint8_t>> level;
    uint64_t type;
    uint64_t entry_create_time;
    uint64_t entry_effective_time;
    uint64_t primary_key() const { return entry_id; }
    uint64_t get_user_name() const { return user.value; }
    uint64_t get_type() const { return type; }
  };
  typedef eosio::multi_index<
      "entryinfor"_n, entryinfor,
      indexed_by<"byuser"_n, const_mem_fun<entryinfor, uint64_t,
                                           &entryinfor::get_user_name>>,
      indexed_by<"bytype"_n,
                 const_mem_fun<entryinfor, uint64_t, &entryinfor::get_type>>>
      entry_index;

      entry_index entryIndex;
};
