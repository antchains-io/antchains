
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
using namespace eosio;

CONTRACT portionset : public eosio::contract {
public:
  using contract::contract;

  portionset(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds), 
      _portion(_self, _self.value) {
         _userportion = _portion.get_or_default();
        }

  ~portionset() {
      _portion.set( _userportion, _self );
  }

  ACTION setportion(int64_t order_portion);

private:
   TABLE portion {
      int64_t order_portion = 0;
   };
   typedef eosio::singleton<"portion"_n, portion> userportion_singleton;
   
   userportion_singleton _portion;
   portion _userportion;

};

