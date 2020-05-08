/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#pragma once
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/redis_plugin/reply_result.hpp>
#include <appbase/application.hpp>
#include <vector>
#include <hiredis/hiredis.h>

namespace eosio {
using namespace appbase;

class redis_plugin : public plugin<redis_plugin> {
 public:
   APPBASE_PLUGIN_REQUIRES()

   redis_plugin();
   virtual ~redis_plugin();

   virtual void set_program_options(options_description&, options_description&) override;
   void plugin_initialize(const variables_map&);
   int redis_init();

   //redis string cmd
   bool redis_set(const string& cmd);
   string redis_get(const string& cmd);
   bool redis_incrby(const string& cmd);

   //redis hash cmd
   bool redis_hset(const string& cmd);
   chain::flat_map<std::string,std::string> redis_hgetall(const string& cmd);
   
   //redis set cmd
   bool redis_sadd(const string& cmd);
   vector<std::string> redis_smembers(const string& cmd);

   //redis publish cmd
   bool redis_publish(const string& cmd);

   //redis common
   void parse_reply_result(redisReply *reply, const std::string &cmd, CReplyResult &result);
   chain::flat_map<std::string,std::string> handle_reply_hash(redisReply *reply);
   vector<std::string> handle_reply_vector(redisReply *reply);

   void plugin_startup();
   void plugin_shutdown();

 private:
   redisContext *m_redis = NULL;
   unique_ptr<redisReply> m_reply;
   CReplyResult m_replyResult;
   std::string ip;
   int port;
};

} // namespace eosio