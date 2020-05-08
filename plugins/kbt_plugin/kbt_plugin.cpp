/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/kbt_plugin/kbt_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <sstream>

#include <fc/io/json.hpp>

namespace eosio {

static appbase::abstract_plugin& _kbt_plugin = app().register_plugin<kbt_plugin>();

using namespace eosio;

// ********** Helper function (copy from main.cpp(cleos) and chain plugin) ******
auto tx_expiration = fc::seconds(30);
const fc::microseconds abi_serializer_max_time = fc::seconds(10);
vector<string> tx_permission;

//copy pasta from keosd's main.cpp
bfs::path determine_home_directory()
{
   bfs::path home;
   struct passwd* pwd = getpwuid(getuid());
   if(pwd) {
      home = pwd->pw_dir;
   }
   else {
      home = getenv("HOME");
   }
   if(home.empty())
      home = "./";
   return home;
}

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


fc::variant json_from_file_or_string(const string& file_or_str, fc::json::parse_type ptype = fc::json::legacy_parser)
{
   regex r("^[ \t]*[\{\[]");
   if ( !regex_search(file_or_str, r) && fc::is_regular_file(file_or_str) ) {
      return fc::json::from_file(file_or_str, ptype);
   } else {
      return fc::json::from_string(file_or_str, ptype);
   }
}


template<typename Api>
auto make_resolver(const Api* api, const fc::microseconds& max_serialization_time) {
   return resolver_factory<Api>::make(api, max_serialization_time);
}

// ********** End Helper function (copy from main.cpp(cleos)) ******
class kbt_plugin_impl {
 public:
   kbt_plugin_impl(controller& db)
      : db(db) {}

   // chain::plugin_interface::channels::irreversible_block::channel_type::handle     _on_irb_handle;
   controller& db;
   string walletpath;
   string walletpasswd;
   string walletname;
   string transaction_ret;
};

kbt_plugin::kbt_plugin() {}
kbt_plugin::~kbt_plugin() {}

void kbt_plugin::set_program_options(options_description&, options_description& cfg) { 
   cfg.add_options()
         ("wallet-path", bpo::value<string>()->default_value("~/eosio-wallet"), "set wallet path" )
         ("wallet-name", bpo::value<string>()->default_value("default"), "set wallet name" )
         ("wallet-passwd", bpo::value<string>(), "set passwd" );
}

void kbt_plugin::plugin_initialize(const variables_map& options) { 
   my.reset(new kbt_plugin_impl(app().get_plugin<chain_plugin>().chain()));
   if (options.count( "wallet-path" )) {
      my->walletpath = options.at( "wallet-path" ).as<string>();
   }
   if (options.count( "wallet-name" )) {
      my->walletname = options.at("wallet-name").as<string>();
   }
   if (options.count( "wallet-passwd" )) {
      my->walletpasswd = options.at("wallet-passwd").as<string>();
   }
}

#define CALL(call_name, http_response_code)                                                                            \
   {                                                                                                                   \
      std::string("/v1/evs/" #call_name), [this](string, string body, url_response_callback cb) mutable {            \
         try {                                                                                                         \
            if (body.empty())                                                                                          \
               body = "{}";                                                                                            \
            auto result = call_name(fc::json::from_string(body).as<kbt_plugin::call_name##_params>());               \
            cb(http_response_code, fc::json::to_string(result));                                                       \
         } catch (...) {                                                                                               \
            http_plugin::handle_exception("evs", #call_name, body, cb);                                              \
         }                                                                                                             \
      }                                                                                                                \
   }

void kbt_plugin::plugin_startup() {
   ilog("starting kbt_plugin");
   
   auto& wallet_mgr = app().get_plugin<wallet_plugin>().get_wallet_manager();
   boost::filesystem::path dir = my->walletpath;
   wallet_mgr.set_dir(dir);
   wallet_mgr.unlock(my->walletname, my->walletpasswd);
}

const chainbase::database& kbt_plugin::db()const { return my->db.db(); }

void kbt_plugin::plugin_shutdown() {}

bytes kbt_plugin::variant_to_bin( const account_name& account, const action_name& action, const fc::variant& action_args_var ) const {
   const auto &d = my->db.db();
   const chain::account_object *code_accnt = d.find<chain::account_object, chain::by_name>(account);
   EOS_ASSERT(code_accnt != nullptr, chain::account_query_exception, "Fail to retrieve account for ${account}", ("account", account) );
   abi_def abi;
   abi_serializer::to_abi(code_accnt->abi, abi);
   abi_serializer as(abi, abi_serializer_max_time);

   auto action_type = as.get_action_type( action );

   return as.variant_to_binary(action_type,action_args_var,abi_serializer_max_time);
}

string kbt_plugin::push_trx_subcommond(signed_transaction& trx, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none ) const {
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

   auto& wallet_mgr = app().get_plugin<wallet_plugin>().get_wallet_manager();

   if ( !wallet_mgr.is_evs_locked(my->walletname) )
   {
      wallet_mgr.unlock(my->walletname, my->walletpasswd);
   }

   //get_info_params info_params;
   chain_apis::read_only::get_info_params stmp;
   auto info = ro_api.get_info(stmp);

   if (trx.signatures.size() == 0) { //can't change txn content if already signed
      trx.expiration = info.head_block_time + tx_expiration;

      // Set tapos, default to last irreversible block if it's not specified by the user
      block_id_type ref_block_id = info.last_irreversible_block_id;
      trx.set_reference_block(ref_block_id);
   }
   
   auto public_keys = wallet_mgr.get_public_keys();

   chain_apis::read_only::get_required_keys_params req_key_params;
   req_key_params.transaction = trx;
   req_key_params.available_keys = public_keys;
   
   auto required_keys_result = ro_api.get_required_keys(req_key_params);

   chain::signed_transaction sig_trx;
   sig_trx = wallet_mgr.sign_transaction(trx, required_keys_result.required_keys, info.chain_id);

   auto rw_api = app().get_plugin<chain_plugin>().get_read_write_api();
   chain_apis::read_write::push_transaction_params push_trx_params;

   auto resolver = make_resolver(this, abi_serializer_max_time);
   fc::variant a(packed_transaction(sig_trx, compression));

   my->transaction_ret = "";
   chain::plugin_interface::next_function<chain_apis::read_write::push_transaction_results> next = 
   [&] (const fc::static_variant<fc::exception_ptr, chain_apis::read_write::push_transaction_results>& result) {
      if (result.contains<fc::exception_ptr>()) {
               try {
                  result.get<fc::exception_ptr>()->dynamic_rethrow_exception();
               } catch (chain::tx_duplicate& e) {
                  // my->transaction_ret = e.what();
               } catch (...) {
                  // my->transaction_ret = "push_trx_subcommond error";
               }
            }
   };

   rw_api.push_transaction_v2(a.get_object(), next);
   return my->transaction_ret;
}

string kbt_plugin::push_actions(std::vector<chain::action>&& actions, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none ) const {
   signed_transaction trx;
   trx.actions = std::forward<decltype(actions)>(actions);
   return push_trx_subcommond(trx, extra_kcpu, compression);
}

vector<chain::permission_level> get_account_permissions(const vector<string>& permissions) {
   auto fixedPermissions = permissions | boost::adaptors::transformed([](const string& p) {
      vector<string> pieces;
      split(pieces, p, boost::algorithm::is_any_of("@"));
      if( pieces.size() == 1 ) pieces.push_back( "active" );
      return chain::permission_level{ .actor = pieces[0], .permission = pieces[1] };
   });
   vector<chain::permission_level> accountPermissions;
   boost::range::copy(fixedPermissions, back_inserter(accountPermissions));
   return accountPermissions;
}

vector<chain::permission_level> get_account_permissions(const vector<string>& permissions, const chain::permission_level& default_permission) {
   if (permissions.empty())
      return vector<chain::permission_level>{default_permission};
   else
      return get_account_permissions(tx_permission);
}

string kbt_plugin::send_action(account_name code, action_name acttype, std::vector<string> auths, const string& data) const {
   auto accountPermissions = get_account_permissions(auths);
   const fc::variant action_args_var = json_from_file_or_string(data, fc::json::relaxed_parser);

   return push_actions({chain::action{accountPermissions, code, acttype, 
                              variant_to_bin( code, acttype, action_args_var ) }});
}

chain::action kbt_plugin::newaccount_action(const chain::name& newaccount, public_key_type keyall) {
   chain::name creator("evscreator");
   return action {
      get_account_permissions(tx_permission, {creator,config::active_name}),
      eosio::chain::newaccount{
         .creator      = creator,
         .name         = newaccount,
         .owner        = eosio::chain::authority{1, {{keyall, 1}}, {}},
         .active       = eosio::chain::authority{1, {{keyall, 1}}, {}}
      }
   };
}

void kbt_plugin::send_actions(std::vector<chain::action>&& actions) {
   int32_t extra_kcpu = 1000;
   packed_transaction::compression_type compression = packed_transaction::none;
   auto result = push_actions( move(actions), extra_kcpu, compression);
}

chain::action kbt_plugin::create_action(const vector<permission_level>& authorization, const account_name& code, const action_name& act, const fc::variant& args) {
   return chain::action{authorization, code, act, variant_to_bin(code, act, args)};
}

chain::action kbt_plugin::create_creator_action(const chain::name& user) {
   chain::name creator("evscreator");
   std::string data ="\[\"" + user.to_string() + "\", \"" + "0" + "\", \"1\"]";
   //data: ["evsmeg", "0", "1"]
   const fc::variant act_payload = json_from_file_or_string(data, fc::json::relaxed_parser);
   auto accountPermissions = get_account_permissions(tx_permission, {creator,config::active_name});
   return create_action(accountPermissions, creator, N(updatecheck), act_payload);
}

// bool kbt_plugin::uid_createaccount(const uid_createaccount_params& params) {
//    bool bret = false;

//    const auto &d = my->db.db();
//    const chain::account_object *code_accnt = d.find<chain::account_object, chain::by_name>( params.new_account );
//    EOS_ASSERT(code_accnt == nullptr, chain::account_name_exists_exception, 
//                "Cannot uid create account named ${name}, as that name is already taken",
//                ("name", params.new_account));

//    auto create = newaccount_action(params.new_account, params.key_pair);
//    auto creator = create_creator_action( params.new_account );
//    send_actions( { create, creator } );
//    return true;
// }

} // namespace eosio
