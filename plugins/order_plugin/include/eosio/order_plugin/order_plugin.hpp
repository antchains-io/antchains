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


class order_plugin : public plugin<order_plugin> {
 public:
   APPBASE_PLUGIN_REQUIRES((redis_plugin)(chain_plugin)(kbt_plugin))

   order_plugin();
   virtual ~order_plugin();

   virtual void set_program_options(options_description&, options_description&) override;
   void plugin_initialize(const variables_map&);
   void plugin_startup();
   void plugin_shutdown();


   const chainbase::database& db()const;
 private:
   uint64_t get_simple_row_by_type(const std::string& code_str, const std::string& scope_str, const std::string& table_str) const;
   void operate_account_handle();
   void block_handler(const signed_block_ptr& block, uint32_t precision, int64_t proportion) const;

   unique_ptr<class order_plugin_impl> my;
};

} // namespace eosio