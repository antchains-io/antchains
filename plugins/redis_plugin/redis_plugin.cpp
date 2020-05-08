/**
 *  @file
 *  @copyright defined in eos/LICENSE
 */
#include <eosio/chain/exceptions.hpp>
#include <eosio/redis_plugin/redis_plugin.hpp>
#include <eosio/chain/exceptions.hpp>

#include <fc/io/json.hpp>

namespace eosio {

static appbase::abstract_plugin& _redis_plugin = app().register_plugin<redis_plugin>();

using namespace eosio;


redis_plugin::redis_plugin() : m_reply(nullptr) {}
redis_plugin::~redis_plugin() {}

void redis_plugin::set_program_options(options_description&, options_description& cfg) { 
     cfg.add_options()
        ("redis-ip", bpo::value<string>()->default_value("127.0.0.1"), "The actual host used to connect redis" )
        ("redis-port", bpo::value<int>()->default_value(6379), "The actual port used to connect redis" )
        ("redis-user", bpo::value<string>()->default_value("root"), "Redis login username" )
        ("redis-passwd", bpo::value<string>()->default_value("123456"), "Redis login passwd" );
}

int redis_plugin::redis_init()
{    
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    redisContext *c = NULL;
    c = redisConnectWithTimeout(ip.c_str(), port, timeout);
    if (NULL == c || c->err) {
        if(c) {
            ilog("Redis Error");
            redisFree(c);
        } else {
            ilog("Redis failure");
        }
        return -1;
    }
    m_redis = c;
    return 0;
}

void redis_plugin::plugin_initialize(const variables_map& options) {
    if (options.count("redis-ip")) {
        ip = options.at( "redis-ip" ).as<string>();
    }
    if (options.count( "redis-port" )) {
        port = options.at( "redis-port" ).as<int>();
    }
    /* init redis */
    EOS_ASSERT( redis_init() == 0, eosio::chain::plugin_config_exception, "Redis init failed");
    EOS_ASSERT( m_redis->err == 0, eosio::chain::plugin_config_exception, "Invalid entry in action-blacklist: '${a}'", ("a", m_redis->errstr));

    //init data
    string set_oper_acc_zero = "set operate_account 0";
    redis_set(set_oper_acc_zero);
}

bool redis_plugin::redis_set(const std::string& cmd) {
	EOS_ASSERT( 0 == cmd.find_first_of("set"), eosio::chain::plugin_config_exception, "set command error : '${s}'", ("s", cmd));
   m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));
   if(m_reply != nullptr && (m_reply->type != REDIS_REPLY_ERROR))
   {
      return true;
   }
   return false;
}

string redis_plugin::redis_get(const std::string& cmd) {
   std::string result = "";
   EOS_ASSERT( 0 == cmd.find_first_of("get"), eosio::chain::plugin_config_exception, "set command error : '${s}'", ("s", cmd));
   m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));

   EOS_ASSERT( m_reply != nullptr, eosio::chain::plugin_config_exception, "redis_get fail Command: '${s}'", ("s", cmd));
	EOS_ASSERT( m_reply->type != REDIS_REPLY_ERROR, eosio::chain::plugin_config_exception, "redis_get m_reply fail, m_reply type '${n}'", ("n", m_reply->type));
   if(m_reply != nullptr && (m_reply->type != REDIS_REPLY_ERROR)) {
      result = m_reply->str;
   }
   return result;
}

bool redis_plugin::redis_incrby(const std::string& cmd) {
	EOS_ASSERT( 0 == cmd.find_first_of("incrby"), eosio::chain::plugin_config_exception, "set command error : '${s}'", ("s", cmd));
   m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));
   if(m_reply != nullptr && (m_reply->type != REDIS_REPLY_ERROR))
   {
      return true;
   }
   return false;
}

bool redis_plugin::redis_hset(const std::string& cmd) {
	EOS_ASSERT( 0 == cmd.find_first_of("hset"), eosio::chain::plugin_config_exception, "hset command error : '${s}'", ("s", cmd));
	m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));
   if(m_reply != nullptr && (m_reply->type != REDIS_REPLY_ERROR)) {
      return true;
   }
   return false;
}

chain::flat_map<std::string,std::string> redis_plugin::redis_hgetall(const string& cmd) {
	EOS_ASSERT( 0 == cmd.find_first_of("hgetall"), eosio::chain::plugin_config_exception, "hgetall command error : '${s}'", ("s", cmd));
	m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));
	
	EOS_ASSERT( m_reply != nullptr, eosio::chain::plugin_config_exception, "hgetall fail Command: '${s}'", ("s", cmd));
	EOS_ASSERT( m_reply->type != REDIS_REPLY_ERROR, eosio::chain::plugin_config_exception, "hgetall m_reply fail, m_reply type '${n}'", ("n", m_reply->type));

   parse_reply_result(m_reply.get(), cmd, m_replyResult);

   return m_replyResult.GetReplyHash();
}

bool redis_plugin::redis_sadd(const string& cmd) {
   EOS_ASSERT(0 == cmd.find_first_of("sadd"), eosio::chain::plugin_config_exception, "sadd command error : '${s}'", ("s", cmd));
   m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));
   if (m_reply != nullptr && (m_reply->type != REDIS_REPLY_ERROR)) {
		return true;
	}
   return false;
}

bool redis_plugin::redis_publish(const string& cmd) {
   EOS_ASSERT(0 == cmd.find_first_of("publish"), eosio::chain::plugin_config_exception, "publish command error : '${s}'", ("s", cmd));
   m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));
   if (m_reply != nullptr && (m_reply->type != REDIS_REPLY_ERROR)) {
		return true;
	}
   return false;
}


vector<std::string> redis_plugin::redis_smembers(const std::string& cmd) {
	EOS_ASSERT( 0 == cmd.find_first_of("smembers"), eosio::chain::plugin_config_exception, "hgetsmembersall command error : '${s}'", ("s", cmd));
	m_reply.reset((redisReply *)redisCommand(m_redis, cmd.c_str()));
	
	EOS_ASSERT( m_reply != nullptr, eosio::chain::plugin_config_exception, "smembers fail Command: '${s}'", ("s", cmd));
	EOS_ASSERT( m_reply->type != REDIS_REPLY_ERROR, eosio::chain::plugin_config_exception, "smembers m_reply fail, m_reply type '${n}'", ("n", m_reply->type));

    parse_reply_result(m_reply.get(), cmd, m_replyResult);

    return m_replyResult.GetReplyVector();
}

void redis_plugin::parse_reply_result(redisReply *reply, const std::string &cmd, CReplyResult &result) {
	result.Clear();
   if (reply->type == REDIS_REPLY_INTEGER) {
      if (cmd.find("exists") != std::string::npos || cmd.find("sismembers") != std::string::npos ||
          cmd.find("hexists") != std::string::npos) {
         if (reply->integer == 1) {
            result.SetReplyBool(true);
            result.SetReplyType(REPLY_BOOLEN);
         } else {
            //ilog("The cmd is '${s}", ("s", cmd.c_str()));
            result.SetReplyBool(false);
            result.SetReplyType(REPLY_BOOLEN);
         }
		} else {
         result.SetReplyInteger(reply->integer);
         result.SetReplyType(REPLY_INTEGER);
      }
	} 
   else if(reply->type == REDIS_REPLY_STRING || reply->type == REDIS_REPLY_STATUS) {
      result.SetReplyString(reply->str);
      result.SetReplyType(REPLY_STRING);
   }
	else if(reply->type == REDIS_REPLY_NIL) {
      result.SetReplyString("");
      result.SetReplyType(REPLY_STRING);
   }
	else if(reply->type == REDIS_REPLY_ARRAY) {
      if (reply->elements == 1 && cmd.find("LRANGE") != std::string::npos) {
         result.SetReplyString(reply->element[0]->str);
         result.SetReplyType(REPLY_STRING);
      } else {
         //Distinguish list and map types
         if (cmd.find("hgetall") != std::string::npos) {
            result.SetReplyHash(handle_reply_hash(reply));
            result.SetReplyType(REPLY_HASH);
         } else {
            result.SetReplyVector(handle_reply_vector(reply));
            result.SetReplyType(REPLY_LIST);
         }
      }
   }
   else if (reply->type == REDIS_REPLY_ERROR) {
      result.SetReplyError(reply->str);
      result.SetReplyType(REPLY_ERROR);
   }
}

//parses reply return type as hash
chain::flat_map<std::string,std::string> redis_plugin::handle_reply_hash(redisReply *reply) {
	chain::flat_map<std::string,std::string> resultMap;
	for(size_t i = 0; i < reply->elements; i += 2)
	{
		resultMap[reply->element[i]->str] = reply->element[i+1]->str;
	}
	return resultMap;
}

//parses reply return type as vector
vector<std::string> redis_plugin::handle_reply_vector(redisReply *reply) {
	vector<std::string> resultVec;
	for(size_t i = 0; i < reply->elements; i++)
	{
		if(reply->element[i]->type == REDIS_REPLY_STRING)
		{
			resultVec.push_back(reply->element[i]->str);
		}
		else if(reply->element[i]->type == REDIS_REPLY_NIL)
		{
			resultVec.push_back("");
		}
	}
	return resultVec;
}

void redis_plugin::plugin_startup() {
   ilog("starting redis_plugin");
}

void redis_plugin::plugin_shutdown() {
    if(m_redis != NULL) {
        redisFree(m_redis);
	}
}

} // namespace eosio
