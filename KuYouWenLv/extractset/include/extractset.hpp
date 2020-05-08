
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
using namespace eosio;

CONTRACT extractset : public eosio::contract {
public:
  using contract::contract;

  extractset(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds), 
      _extract(_self, _self.value) {
         _userextract = _extract.get_or_default();
        }

  ~extractset() {
      _extract.set( _userextract, _self );
  }

  ACTION setpextract(uint64_t extract_time_hour);

private:
   TABLE extracttime {
      uint64_t extract_time_hour = 0;
   };
   typedef eosio::singleton<"extracttime"_n, extracttime> userprecision_singleton;
   
   userprecision_singleton _extract;
   extracttime _userextract;
};
