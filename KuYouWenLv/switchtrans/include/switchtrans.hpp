
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
using namespace eosio;

CONTRACT switchtrans : public eosio::contract {
public:
  using contract::contract;

  switchtrans(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds), 
      _swtichstate(_self, _self.value) {
         _userstate = _swtichstate.get_or_default();
        }

  ~switchtrans() {
      _swtichstate.set( _userstate, _self );
  }

  ACTION settrans(uint8_t switch_state);

private:
   TABLE swstate {
      uint8_t switch_state = 0;
   };
   typedef eosio::singleton<"swstate"_n, swstate> switchstate_singleton;
   
   switchstate_singleton _swtichstate;
   swstate _userstate;
};

