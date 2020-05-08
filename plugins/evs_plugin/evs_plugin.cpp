/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/evs_plugin/evs_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <sstream>

#include <fc/io/json.hpp>

namespace eosio {

static appbase::abstract_plugin& _evs_plugin = app().register_plugin<evs_plugin>();

using namespace eosio;

class evs_plugin_impl {
 public:
   evs_plugin_impl(controller& db)
      : db(db) {}

   controller& db;
};

evs_plugin::evs_plugin() {}
evs_plugin::~evs_plugin() {}

void evs_plugin::set_program_options(options_description&, options_description& cfg) { }

void evs_plugin::plugin_initialize(const variables_map& options) { 
   my.reset(new evs_plugin_impl(app().get_plugin<chain_plugin>().chain()));
}

#define CALL(call_name, http_response_code)                                                                            \
   {                                                                                                                   \
      std::string("/v1/evs/" #call_name), [this](string, string body, url_response_callback cb) mutable {            \
         try {                                                                                                         \
            if (body.empty())                                                                                          \
               body = "{}";                                                                                            \
            auto result = call_name(fc::json::from_string(body).as<evs_plugin::call_name##_params>());               \
            cb(http_response_code, fc::json::to_string(result));                                                       \
         } catch (...) {                                                                                               \
            http_plugin::handle_exception("evs", #call_name, body, cb);                                              \
         }                                                                                                             \
      }                                                                                                                \
   }

void evs_plugin::plugin_startup() {
   ilog("starting evs_plugin");
 
   app().get_plugin<http_plugin>().add_api({
      CALL(get_all_producers, 200),
      CALL(get_token_exch_ratio, 200),
   });
}

const chainbase::database& evs_plugin::db()const { return my->db.db(); }

void evs_plugin::plugin_shutdown() {}

evs_plugin::get_all_producers_result evs_plugin::get_all_producers(const get_all_producers_params&) const {
    return static_cast<get_all_producers_result>(my->db.active_producers());
}

evs_plugin::get_token_exch_ratio_result evs_plugin::get_token_exch_ratio(const get_token_exch_ratio_params& params) const {
    get_token_exch_ratio_result result;
    chain_apis::read_only::get_table_rows_params table_params;
    table_params.json = true;
    table_params.code = "evsio.token";
    table_params.scope = "evsio.token";
    table_params.table = "tokeneval";
    table_params.table_key = "";
    table_params.lower_bound = params.tokenFrom;
    table_params.upper_bound = "";
    table_params.limit = 1;
    table_params.key_type = "";
    table_params.index_position = "";
    table_params.encode_type = "dec";
    table_params.reverse = false;
    table_params.show_payer = false;

    token_evals from_ret = get_token_eval_of(table_params);
    table_params.lower_bound = params.tokenTo;
    token_evals to_ret = get_token_eval_of(table_params);

    result.num = from_ret.total().value;
    result.den = to_ret.total().value;

    return result;
}

evs_plugin::token_evals evs_plugin::get_token_eval_of(const chain_apis::read_only::get_table_rows_params &params) const{
    token_evals result;
    auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
    const auto ret = ro_api.get_table_rows(params);
    EOS_ASSERT(ret.rows.size() > 0, chain::plugin_exception, "token ${from} no market value set", ("from", params.lower_bound));
    std::string re = fc::json::to_string(ret.rows[0]);
    const char* number_str = "123456789";
    const char* number_str2 = "1234567890";
    auto value_pos_start = re.find_first_of(number_str);
    auto value_pos_end = re.find_first_not_of(number_str2, value_pos_start);
    auto user_pos_start = re.find_first_of(number_str, value_pos_end);
    auto user_pos_end = re.find_first_not_of(number_str2, user_pos_start);
    auto order_pos_start = re.find_first_of(number_str, user_pos_end);
    auto order_pos_end = re.find_first_not_of(number_str2, order_pos_start);
    auto trans_pos_start = re.find_first_of(number_str, order_pos_end);
    auto trans_pos_end = re.find_first_not_of(number_str2, trans_pos_start);
    EOS_ASSERT( value_pos_start != std::string::npos && value_pos_end != std::string::npos &&
                user_pos_start != std::string::npos && user_pos_end != std::string::npos &&
                order_pos_start != std::string::npos && order_pos_end != std::string::npos &&
                trans_pos_start != std::string::npos && trans_pos_end != std::string::npos,
        eosio::chain::plugin_exception, "get_token_eval_of err.");
    result.value = fc::to_uint64(re.substr(value_pos_start, value_pos_end - value_pos_start));
    result.user_volume = fc::to_uint64(re.substr(user_pos_start, user_pos_end - user_pos_start));
    result.order_volume = fc::to_uint64(re.substr(order_pos_start, order_pos_end - order_pos_start));
    result.trans_volume = fc::to_uint64(re.substr(trans_pos_start, trans_pos_end - trans_pos_start));
    return result;
}

} // namespace eosio
