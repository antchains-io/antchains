#include <portionset.hpp>

ACTION portionset::setportion(int64_t set_portion)
{
   require_auth(get_self());
   eosio_assert( set_portion >= 1 && set_portion < 10000, "set proportion min 1 and max 10000." );
   _userportion.order_portion = set_portion;
}
EOSIO_DISPATCH( portionset, (setportion))
