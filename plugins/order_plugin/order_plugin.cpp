/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/order_plugin/order_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <sstream>

#include <fc/io/json.hpp>

namespace eosio {

static appbase::abstract_plugin& _order_plugin = app().register_plugin<order_plugin>();

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
class order_plugin_impl {
 public:
   order_plugin_impl(controller& db)
      : db(db) {}

   chain::plugin_interface::channels::irreversible_block::channel_type::handle     _on_irb_handle;
   controller& db;
   kbt_plugin* kbt_plug = nullptr;
};

order_plugin::order_plugin() {}
order_plugin::~order_plugin() {}

void order_plugin::set_program_options(options_description&, options_description& cfg) {}

void order_plugin::plugin_initialize(const variables_map& options) { 
   my.reset(new order_plugin_impl(app().get_plugin<chain_plugin>().chain()));
}

void order_plugin::plugin_startup() {
   ilog("starting order_plugin");
   my->kbt_plug = app().find_plugin<kbt_plugin>();
   
   my->_on_irb_handle = app().get_channel<chain::plugin_interface::channels::irreversible_block>()
                .subscribe( [this]( eosio::chain::block_state_ptr s ){
                     //1days ->1*24*60*60 sec ->1*24*60*60*2 height
                     //test for min
                     uint32_t set_time_height = get_simple_row_by_type("extractset", "extractset", "extracttime") * 60 * 2;
                     uint32_t precision = get_simple_row_by_type("precisionset", "precisionset", "precision");
                     int64_t proportion =  get_simple_row_by_type("portionset", "portionset", "portion");
                     uint32_t current_height = block_header::num_from_id(s->block->id());
                     
                     string get_cmd = "get operate_account";
                     std::string dest_amount_str = app().get_plugin<redis_plugin>().redis_get(get_cmd);
                     
                     if (current_height <= set_time_height) {
                        ilog("jerry ====== dest_height: ${re} , current_height: ${cu} ,precision : ${pr}", ("re", set_time_height)("cu", current_height)("pr", precision));
                     }
                     else
                     {
                        uint32_t tmp = current_height / set_time_height;
                        uint32_t new_height_tmp = set_time_height*(tmp + 1);
                        ilog("set_height : ${rw} , dest_height: ${re} , current_height: ${cu} ,precision : ${pr} , wait_extract_amount : ${pt}", ("rw", set_time_height)("re", new_height_tmp)("cu", current_height)("pr", precision)("pt", dest_amount_str));
                     }
                  
                     if (current_height >= set_time_height) {
                        uint32_t new_height = current_height + set_time_height;
                        if ( (new_height % set_time_height) == 0 ) {
                           operate_account_handle();
                        }
                     }
                     block_handler(s->block, precision, proportion);
                     // ilog("evs block_num ${block_num}, dpos_proposed_irreversible_blocknum: ${a}, dpos_irreversible_blocknum: ${b}, bft_irreversible_blocknum: ${c}, pending_schedule_lib_num: ${d}", 
                           // ("block_num", s->block_num)("a",s->dpos_proposed_irreversible_blocknum)("b", s->dpos_irreversible_blocknum)("c", s->bft_irreversible_blocknum)("d", s->pending_schedule_lib_num));
                });
   // app().get_plugin<http_plugin>().add_api({
	//   CALL(uid_createaccount, 200), 
   // });
}

const chainbase::database& order_plugin::db()const { return my->db.db(); }

uint64_t order_plugin::get_simple_row_by_type(const std::string& code_str, const std::string& scope_str, const std::string& table_str) const {
   chain_apis::read_only::get_table_rows_params table_params;
   table_params.json = true;
   table_params.code = code_str;
   table_params.scope = scope_str;
   table_params.table = table_str;
   table_params.table_key = "";
   table_params.lower_bound = "";
   table_params.upper_bound = "";
   table_params.limit = 1;
   table_params.key_type = "";
   table_params.index_position = "";
   table_params.encode_type = "dec";
   table_params.reverse = false;
   table_params.show_payer = false;

   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();
   const auto ret = ro_api.get_table_rows(table_params);

   EOS_ASSERT(ret.rows.size() > 0, chain::plugin_exception, "no value in ${contract} contract of ${action} action .", ("contract", code_str)("action", table_str));

   std::string re = fc::json::to_string(ret.rows[0]);
   const char* number_str = "123456789";
   const char* number_str2 = "1234567890";
   auto value_pos_start = re.find_first_of(number_str);
   auto value_pos_end = re.find_first_not_of(number_str2, value_pos_start);
   uint64_t value = fc::to_uint64(re.substr(value_pos_start, value_pos_end - value_pos_start));
   return value;
}

void order_plugin::block_handler(const signed_block_ptr& block, uint32_t precision, int64_t proportion) const {
   fc::variant pretty_output;
   abi_serializer::to_variant(*block, pretty_output, make_resolver(this, abi_serializer_max_time), abi_serializer_max_time);
   const variant &transactions = pretty_output["transactions"];
   int64_t money_prec = 100;
   int64_t redis_return = 0;
   for (size_t i = 0; i < transactions.size(); ++i) {
      const variant &actions = transactions[i]["trx"]["transaction"]["actions"];
      for (size_t j = 0; j < actions.size(); ++j) {
         const variant &action = actions[j];
         const variant &data = action["data"];
         uint64_t type = data["type"].as_uint64();
         std::string user = data["user"].as_string();
         chain::name user_name(user);
         //user account bind_check < 2, can return kbt
         const auto& user_account = my->db.db().get<account_object, by_name>(user_name);
         uint8_t account_check = user_account.bind_check;
         if ( (action["account"].as_string() == "kuyouwenlv") && (action["name"].as_string() == "updatetype") && (type == 2) && (account_check < 2) ) {
            std::string amount_str = data["order_amount"].as_string();
            int64_t amount = boost::lexical_cast<int64_t>(amount_str.c_str(), amount_str.size());

            //int64_t proportion = 100;//get from contract interface
	         int64_t money_prec = 10000 / proportion;
            //amount;									                                             //1024'5100 -> 1024.51
            int64_t need_return = amount / money_prec;					                           //102451 -> 10.2451

            if(amount < money_prec * 10000)
            {
               redis_return = need_return;
            }
            else
            {
               int64_t fourth_place = need_return / 1000 % 10;
               int64_t third_place = need_return / 100 % 10;
               int64_t second_place = need_return / 10 % 10;
               int64_t first_place = need_return % 10;
               if (precision == 1000) {
                  redis_return = fourth_place * 1000 + third_place * 100 + second_place * 10 + first_place;    //2451 -> 0.2451
               }
               else if (precision == 100) {
                  redis_return = third_place * 100 + second_place * 10 + first_place;    //451 -> 0.0451
               }
            }
            
            int64_t real_return = need_return - redis_return;					                  //10'2000 ->10.2
            ilog("jerry : block_handler need_return ${s} redis_return ${t} real_return ${d}", ("s", need_return)("t", redis_return)("d", real_return));
            //1024         102.4       10.24       1.024
            //1024,0000    102,4000    10,2400     1,0240

            // 0.1   0.01     0.001    0.0001
            //1000   100      10       1
            //transfer to oper_account
            string incrby_cmd = "incrby operate_account ";
            incrby_cmd += fc::to_string(redis_return);
            app().get_plugin<redis_plugin>().redis_incrby(incrby_cmd);
            //transfer to user_acc
            chain::asset quanlity(real_return, chain::symbol(chain::string_to_symbol_c(4,"KBT")));
            //"\[\"mine\", \"evsjerry\", \"1024.0000 KBT\", \"hello jerry\"]";
            my->kbt_plug->send_action(N(evsio.token), N(transfer), std::vector<string>{"mine@active"}, 
               "\[\"mine\", \"" + user + "\", \"" + quanlity.to_string() + "\", \"hello jerry\"]");
         }
      }
   }
}

void order_plugin::operate_account_handle() {
   ilog("jerry : do operate_account_handle while time to upset");
   string get_cmd = "get operate_account";
   std::string dest_amount_str = app().get_plugin<redis_plugin>().redis_get(get_cmd);
   int64_t dest_amount = boost::lexical_cast<int64_t>(dest_amount_str.c_str(), dest_amount_str.size());
   chain::asset dest_quanlity(dest_amount, chain::symbol(chain::string_to_symbol_c(4,"KBT")));
   my->kbt_plug->send_action(N(evsio.token), N(transfer), std::vector<string>{"mine@active"}, 
         "\[\"mine\", \"evsoper\", \"" + dest_quanlity.to_string() + "\", \"hello jerry\"]");
   
   string set_oper_acc_zero = "set operate_account 0";
   app().get_plugin<redis_plugin>().redis_set(set_oper_acc_zero);
}


void order_plugin::plugin_shutdown() {}

} // namespace eosio
