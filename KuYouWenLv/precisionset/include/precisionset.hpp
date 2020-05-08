
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
using namespace eosio;

CONTRACT precisionset : public eosio::contract {
public:
  using contract::contract;

  precisionset(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds), 
      _precision(_self, _self.value) {
         _userprecision = _precision.get_or_default();
        }

  ~precisionset() {
      _precision.set( _userprecision, _self );
  }

  ACTION setprecision(uint64_t order_precision);

private:
   TABLE precision {
      uint64_t order_precision = 0;
   };
   typedef eosio::singleton<"precision"_n, precision> userprecision_singleton;
   
   userprecision_singleton _precision;
   precision _userprecision;

};

