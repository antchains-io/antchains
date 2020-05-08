#include <eosiolib/eosio.hpp>
using namespace eosio;

CONTRACT evscreator : public eosio::contract {
public:
  using contract::contract;

  evscreator(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds) {}

  ACTION updatecheck(name user, uint8_t from, uint8_t to);
  ACTION updatevalue(name user, uint64_t to);

private:

};
