
#include <eosiolib/eosio.hpp>
#include <math.h>
using namespace eosio;

CONTRACT kuyouwenlv : public eosio::contract {

public:
   using contract::contract;

   enum SUBJECTYPE { COMMENT, ROADBOOK, AIRTICKETS, HOTEL, OTHER };

   kuyouwenlv(name receiver, name code, datastream<const char *> ds)
         : contract(receiver, code, ds),
         kuyouwenlvIndex(_self, _self.value) {}

   ACTION insert(name user, uint8_t type, uint64_t primarykey,
                  uint64_t order_amount);
   ACTION updatetype(name user, uint8_t type, uint64_t primarykey,
                uint64_t order_amount);
   ACTION erase(uint64_t primarykey);
   ACTION reqkywl(uint64_t primarykey);
   
   using insert_action = action_wrapper<"insert"_n, &kuyouwenlv::insert>;
   using updatetype_action = action_wrapper<"updatetype"_n, &kuyouwenlv::updatetype>;
   using erase_action = action_wrapper<"erase"_n, &kuyouwenlv::erase>;
   using reqkywl_action = action_wrapper<"reqkywl"_n, &kuyouwenlv::reqkywl>;

private:
  TABLE kuyouwenlvs {
    name user;
    uint64_t type;
    uint64_t primarykey;
    uint64_t order_amount;

    uint64_t primary_key() const { return primarykey; }
    uint64_t get_user_name() const { return user.value; }
    uint64_t get_type() const { return type; }
  };

  typedef eosio::multi_index<
      "kuyouwenlvs"_n, kuyouwenlvs,
      indexed_by<"byuser"_n, const_mem_fun<kuyouwenlvs, uint64_t,
                                           &kuyouwenlvs::get_user_name>>,
      indexed_by<"bytype"_n,
                 const_mem_fun<kuyouwenlvs, uint64_t, &kuyouwenlvs::get_type>>>
      kuyouwenlvs_index;

  kuyouwenlvs_index kuyouwenlvIndex;
};
