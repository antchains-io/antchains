/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/evo_plugin/evo_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <sstream>

#include <fc/io/json.hpp>

namespace eosio {

static appbase::abstract_plugin& _evo_plugin = app().register_plugin<evo_plugin>();

using namespace eosio;
// ********** Helper function (copy from main.cpp(cleos) and chain plugin) ******
const fc::microseconds abi_serializer_max_time = fc::seconds(10);

template<typename Api>
struct resolver_factory {
   static auto make(const Api* api, const fc::microseconds& max_serialization_time) {
      return [api, max_serialization_time](const account_name &name) -> optional<abi_serializer> {
         const auto* accnt = api->db().template find<chain::account_object, by_name>(name);
         if (accnt != nullptr) {
            abi_def abi;
            if (abi_serializer::to_abi(accnt->abi, abi)) {
               return abi_serializer(abi, max_serialization_time);
            }
         }

         return optional<abi_serializer>();
      };
   }
};

template<typename Api>
auto make_resolver(const Api* api, const fc::microseconds& max_serialization_time) {
   return resolver_factory<Api>::make(api, max_serialization_time);
}

// ********** End Helper function (copy from main.cpp(cleos)) ******
class evo_plugin_impl {
 public:
   evo_plugin_impl(controller& db)
      : db(db) {}

   chain::plugin_interface::channels::irreversible_block::channel_type::handle     _on_irb_handle;
   controller& db;
   kbt_plugin* kbt_plug = nullptr;
};

evo_plugin::evo_plugin() {}
evo_plugin::~evo_plugin() {}

void evo_plugin::set_program_options(options_description&, options_description& cfg) {}

void evo_plugin::plugin_initialize(const variables_map& options) { 
   my.reset(new evo_plugin_impl(app().get_plugin<chain_plugin>().chain()));
}

#define CALL(call_name, http_response_code)                                                                            \
{                                                                                                                   \
   std::string("/v1/evs/" #call_name), [this](string, string body, url_response_callback cb) mutable {            \
      try {                                                                                                         \
         if (body.empty())                                                                                          \
            body = "{}";                                                                                            \
         auto result = call_name(fc::json::from_string(body).as<evo_plugin::call_name##_params>());               \
         cb(http_response_code, fc::json::to_string(result));                                                       \
      } catch (...) {                                                                                               \
         http_plugin::handle_exception("evo_plugin", #call_name, body, cb);                                              \
      }                                                                                                             \
   }                                                                                                                \
}


void evo_plugin::plugin_startup() {
   ilog("starting evo_plugin");
   my->kbt_plug = app().find_plugin<kbt_plugin>();
   
   my->_on_irb_handle = app().get_channel<chain::plugin_interface::channels::irreversible_block>()
                .subscribe( [this]( eosio::chain::block_state_ptr s ){
                     block_handler(s->block);
                     // ilog("evs block_num ${block_num}, dpos_proposed_irreversible_blocknum: ${a}, dpos_irreversible_blocknum: ${b}, bft_irreversible_blocknum: ${c}, pending_schedule_lib_num: ${d}", 
                           // ("block_num", s->block_num)("a",s->dpos_proposed_irreversible_blocknum)("b", s->dpos_irreversible_blocknum)("c", s->bft_irreversible_blocknum)("d", s->pending_schedule_lib_num));
                });

   app().get_plugin<http_plugin>().add_api({
	  CALL(uid_createaccount, 200), 
     CALL(eip2evo, 200), 
   });
}

const chainbase::database& evo_plugin::db()const { return my->db.db(); }

void evo_plugin::block_handler(const signed_block_ptr& block) const {
   fc::variant pretty_output;
   
   abi_serializer::to_variant(*block, pretty_output, make_resolver(this, abi_serializer_max_time), abi_serializer_max_time);
   const variant &transactions = pretty_output["transactions"];
   int64_t money_prec = 100;
   int64_t redis_return = 0;
   size_t t_size = transactions.size();
   for (size_t i = 0; i < t_size; i++) {
   const variant &actions = transactions[i]["trx"]["transaction"]["actions"];
   std::string redis_pub_txid_cmd = "";
   std::string trx_id = transactions[i]["trx"]["id"].as_string();
   size_t a_size = actions.size();
      for (size_t j = 0; j < a_size; j++) {
         const variant &action = actions[j];
         const variant &act_name = action["name"];
         const variant &data = action["data"];
         
         if ( act_name.as_string() == "evo2eip" )
         {
            redis_pub_txid_cmd += "publish ";
            redis_pub_txid_cmd += "txid_key ";
            redis_pub_txid_cmd += "888";
            redis_pub_txid_cmd += "-";
            redis_pub_txid_cmd += trx_id;
            redis_pub_txid_cmd += "-";
            redis_pub_txid_cmd += data["from"].as_string();
            redis_pub_txid_cmd += "-";
            redis_pub_txid_cmd += data["to"].as_string();
            redis_pub_txid_cmd += "-1";
         }
         else if ( act_name.as_string() == "eip2evo" )
         {
            std::string memo = data["memo"].as_string();
            redis_pub_txid_cmd += "publish ";
            redis_pub_txid_cmd += "txid_key ";
            redis_pub_txid_cmd += memo;
            redis_pub_txid_cmd += "-";
            redis_pub_txid_cmd += trx_id;
            redis_pub_txid_cmd += "-";
            redis_pub_txid_cmd += data["from"].as_string();
            redis_pub_txid_cmd += "-";
            redis_pub_txid_cmd += data["to"].as_string();
            redis_pub_txid_cmd += "-2";
         }
         if(0x0!=redis_pub_txid_cmd.size())
         {
            app().get_plugin<redis_plugin>().redis_publish(redis_pub_txid_cmd);
            //ilog( "jerry : redis_pub_txid_cmd ${s}", ("s", redis_pub_txid_cmd) );
         }
      }
   }
}

fc::variant evo_plugin::uid_createaccount(const uid_createaccount_params& params) {
   bool bret = false;

   const auto &d = my->db.db();
   const chain::account_object *code_accnt = d.find<chain::account_object, chain::by_name>( params.new_account );
   EOS_ASSERT(code_accnt == nullptr, chain::account_name_exists_exception, 
               "Cannot uid create account named ${name}, as that name is already taken",
               ("name", params.new_account));

   auto create = my->kbt_plug->newaccount_action(params.new_account, params.key_pair);
   auto creator = my->kbt_plug->create_creator_action( params.new_account );
   my->kbt_plug->send_actions( { create, creator } );
   return fc::json::from_string("{\"code\":200,\"message\":\"success\",\"error\":\"\"}");
}

fc::variant evo_plugin::eip2evo( const eip2evo_params& params ) {
   string s = fc::trim(params.quanlity.to_string());
   // Find space in order to split amount and symbol
   auto space_pos = s.find(' ');
   EOS_ASSERT((space_pos != string::npos), chain::plugin_exception, "Asset's amount and symbol should be separated with space");
   auto symbol_str = fc::trim(s.substr(space_pos + 1));
   auto tgcmine_assert = get_currency_balance_by_symbol( symbol_str );
   
   EOS_ASSERT(( tgcmine_assert >= params.quanlity ), chain::plugin_exception, "tgcmine value is not enough");

   public_key_type key;
   auto time1 = fc::time_point::now();
   // EOS_ASSERT((time_point_sec( fc::time_point::from_iso_string(params.time_value) ) + 30) >= time_point_sec(time1), chain::expired_tx_exception,"the eip2evo transaction has expired");
   const char *pe = params.evo.to_string().c_str();
   const char *pe1 = params.time_value.c_str();
   fc::sha256::encoder enc;
   enc.reset();
   enc.write(pe,strlen(pe));
   enc.write(pe1,strlen(pe1));
   const digest_type digest = enc.result();
   key = public_key_type( params.sig, digest);
   auto permission= my->db.get_authorization_manager().get_permission({N(evochecker), config::active_name});
   EOS_ASSERT( 0x0 != permission.auth.keys.size(), chain::permission_query_exception, "the active perssion of account evochecker is missing");
   auto p = permission.auth.keys.begin();
   for( ; p != permission.auth.keys.end(); p++)
   {
   if(key == p->key)
      break;
   }
   EOS_ASSERT(p!=permission.auth.keys.end(),chain::permission_query_exception,"the public key of sig is not match account tgcmine");
   my->kbt_plug->send_action(N(evsio.token), N(eip2evo), std::vector<string>{"tgcmine@active"}, 
            "\[\"tgcmine\", \"" + params.evo.to_string() + "\", \"" + params.quanlity.to_string() + "\", \"" + params.rand_num + "\"]");
   return fc::json::from_string("{\"code\":200,\"message\":\"success\",\"error\":\"\"}");
}

asset evo_plugin::get_currency_balance_by_symbol( const std::string& symbol_str ) const {
   chain_apis::read_only::get_currency_balance_params currency_balance_params;
   currency_balance_params.code = N(evsio.token);
   currency_balance_params.account = N(tgcmine);
   currency_balance_params.symbol = symbol_str;

   auto ro_api_balance = app().get_plugin<chain_plugin>().get_read_only_api();
   const auto vec_cur_balance = ro_api_balance.get_currency_balance(currency_balance_params);
   EOS_ASSERT(vec_cur_balance.size() > 0, chain::plugin_exception, "no value in tgcmine of ${symbol}.", ("symbol", symbol_str));

   return vec_cur_balance[0];
}

void evo_plugin::plugin_shutdown() {}

} // namespace eosio
