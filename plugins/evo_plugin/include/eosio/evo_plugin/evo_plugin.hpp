/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/kbt_plugin/kbt_plugin.hpp>
#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/redis_plugin/redis_plugin.hpp>
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
   using std::regex;


class evo_plugin : public plugin<evo_plugin> {
 public:
   APPBASE_PLUGIN_REQUIRES((redis_plugin)(chain_plugin)(http_plugin)(kbt_plugin))

   evo_plugin();
   virtual ~evo_plugin();

   virtual void set_program_options(options_description&, options_description&) override;
   void plugin_initialize(const variables_map&);
   void plugin_startup();
   void plugin_shutdown();

   struct uid_createaccount_params {
      chain::name  new_account;
      public_key_type key_pair;
   };
   fc::variant uid_createaccount(const uid_createaccount_params& params);

   struct eip2evo_params {
      account_name evo;
      asset quanlity;
      std::string time_value;
      signature_type sig;
      std::string rand_num;
   };
   fc::variant eip2evo(const eip2evo_params& params);

   const chainbase::database& db()const;
 private:
   void operate_account_handle();
   void block_handler(const signed_block_ptr& block) const;
   asset get_currency_balance_by_symbol( const std::string& symbol_str ) const;

   unique_ptr<class evo_plugin_impl> my;
};

} // namespace eosio

FC_REFLECT(eosio::evo_plugin::uid_createaccount_params, (new_account)(key_pair) )
FC_REFLECT(eosio::evo_plugin::eip2evo_params, (evo)(quanlity)(time_value)(sig)(rand_num) )