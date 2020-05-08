#include <precisionset.hpp>

ACTION precisionset::setprecision(uint64_t set_precision)
{
   require_auth(get_self());
   eosio_assert( set_precision >= 1 && set_precision <= 1000, "set precision min 1 and max 1000." );
   _userprecision.order_precision = set_precision;
}
EOSIO_DISPATCH( precisionset, (setprecision))
