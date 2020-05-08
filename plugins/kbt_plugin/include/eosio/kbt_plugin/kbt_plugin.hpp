/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/wallet_plugin/wallet_plugin.hpp>
#include <eosio/wallet_plugin/wallet_manager.hpp>
#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/chain/account_object.hpp>
#include <appbase/application.hpp>
#include <eosio/chain/controller.hpp>

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


class kbt_plugin : public plugin<kbt_plugin> {
 public:
   APPBASE_PLUGIN_REQUIRES((chain_plugin)(http_plugin)(wallet_plugin))

   kbt_plugin();
   virtual ~kbt_plugin();

   virtual void set_program_options(options_description&, options_description&) override;
   void plugin_initialize(const variables_map&);
   void plugin_startup();
   void plugin_shutdown();


   const chainbase::database& db()const;

   // struct uid_createaccount_params {
   //    chain::name  new_account;
   //    public_key_type key_pair;
   // };
   // bool uid_createaccount(const uid_createaccount_params& params);

   // uint64_t get_simple_row_by_type(const std::string& code_str, const std::string& scope_str, const std::string& table_str) const;
   // void operate_account_handle();
   string send_action( account_name code, action_name acttype, std::vector<string> auths, const string& data ) const;
   void send_actions(std::vector<chain::action>&& actions);
   chain::action create_creator_action(const chain::name& user);
   chain::action create_action(const vector<permission_level>& authorization, const account_name& code, const action_name& act, const fc::variant& args);
   chain::action newaccount_action(const chain::name& newaccount, public_key_type keyall);
   // void block_handler(const signed_block_ptr& block, uint32_t precision, int64_t proportion) const;
 private:
   uint64_t string_to_uint64(const std::string &str) const;
   bytes variant_to_bin( const account_name& account, const action_name& action, const fc::variant& action_args_var ) const;

   string push_actions(std::vector<chain::action>&& actions, int32_t extra_kcpu, packed_transaction::compression_type compression) const;
   string push_trx_subcommond(signed_transaction& trx, int32_t extra_kcpu, packed_transaction::compression_type compression) const;
   unique_ptr<class kbt_plugin_impl> my;
};

} // namespace eosio

// FC_REFLECT(eosio::kbt_plugin::uid_createaccount_params, (new_account)(key_pair) )