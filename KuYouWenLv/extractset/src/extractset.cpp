#include <extractset.hpp>

ACTION extractset::setpextract(uint64_t extract_time_hour)
{
   require_auth(get_self());
   eosio_assert( extract_time_hour >= 1 && extract_time_hour <= 168, "extract time min 1 hour 7 days." );
   _userextract.extract_time_hour = extract_time_hour;
}
EOSIO_DISPATCH( extractset, (setpextract))