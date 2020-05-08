/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/chain/account_object.hpp>
#include <appbase/application.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/symbol.hpp>

#include <fc/static_variant.hpp>
#include <fc/safe.hpp>
#include <regex>
#include <vector>
#include <pwd.h>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace fc { class variant; }

namespace eosio {

   using namespace chain;


class evs_plugin : public plugin<evs_plugin> {
 public:
   APPBASE_PLUGIN_REQUIRES((chain_plugin)(http_plugin))

   evs_plugin();
   virtual ~evs_plugin();

   virtual void set_program_options(options_description&, options_description&) override;
   void plugin_initialize(const variables_map&);
   void plugin_startup();
   void plugin_shutdown();

   struct empty{};

   using get_all_producers_params = empty;
   using get_transaction_sequence_params = empty;
   using get_all_producers_result = eosio::chain::producer_schedule_type;

	struct get_token_exch_ratio_params {
      std::string tokenFrom;
      std::string tokenTo;
   };

   struct get_token_exch_ratio_result {
      uint64_t num;
      uint64_t den;
   };

   get_all_producers_result get_all_producers(const get_all_producers_params&) const;
   get_token_exch_ratio_result get_token_exch_ratio(const get_token_exch_ratio_params&) const;

   const chainbase::database& db()const;

 private:
    struct token_evals {
       uint64_t value;
       uint64_t user_volume;
       uint64_t order_volume;
       uint64_t trans_volume;
       fc::safe<uint64_t> total() {
          fc::safe<uint64_t> _value = value;
          fc::safe<uint64_t> _user_volume = user_volume;
          fc::safe<uint64_t> _order_volume = order_volume;
          fc::safe<uint64_t> _trans_volume = trans_volume;
          return _value + _user_volume + _order_volume + _trans_volume;
       }
    };
   token_evals get_token_eval_of(const chain_apis::read_only::get_table_rows_params &) const;

   unique_ptr<class evs_plugin_impl> my;
};

} // namespace eosio

FC_REFLECT(eosio::evs_plugin::empty, )
FC_REFLECT(eosio::evs_plugin::get_token_exch_ratio_params, (tokenFrom)(tokenTo) )
FC_REFLECT(eosio::evs_plugin::get_token_exch_ratio_result, (num)(den) )