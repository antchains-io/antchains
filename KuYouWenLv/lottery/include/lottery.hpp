#include <eosiolib/eosio.hpp>
#include <eosiolib/action.h>
#include <eosiolib/action.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <math.h>

using namespace std;
using namespace eosio;

#define system_lottery_fee     "blotteryfee"_n //合约运行方收取的手续费用，按照3%收取
CONTRACT lottery : public eosio::contract {
   
public:
  using contract::contract;
  lottery(name receiver, name code, datastream<const char *> ds)
      : contract(receiver, code, ds), bIndex(_self, _self.value),bPool(_self, _self.value) {}

  static time_point_sec from_iso_string( const fc::string& s);
  ACTION bgenerate(uint64_t id,name user,std::string players[2],double odds[2],std::string start_time,
                                   std::string end_time,asset original_prize);
  ACTION bbet(uint64_t id ,uint64_t bet_id,name user,std::string player,std::string bet_time,asset amount);
  ACTION bfreeze(uint64_t id,name user);
  ACTION bcancle(uint64_t id);
  ACTION blottery(uint64_t id,name user,unsigned char result[2]);
  ACTION bredemption(uint64_t id,uint64_t bnet_id,name user);
  ACTION brefund(uint64_t id,uint64_t bnet_id,uint64_t user);

  using generate_action = action_wrapper<"bgenerate"_n, &lottery::bgenerate>;
  using bet_action = action_wrapper<"bbet"_n, &lottery::bbet>;
  using freeze_action = action_wrapper<"bfreeze"_n, &lottery::bfreeze>;
  using cancle_action = action_wrapper<"bcancle"_n, &lottery::bcancle>;
  using lottery_action = action_wrapper<"blottery"_n, &lottery::blottery>;
  using refund_action = action_wrapper<"brefund"_n, &lottery::brefund>;
  using redemption_action = action_wrapper<"bredemption"_n, &lottery::bredemption>;
 
private:
  TABLE b_games{

     uint64_t id;
     name  user;//竞猜创造者,代币需要转向指定的账户，保证各参与方的利益
     std::string players[2];//对战双方信息
     unsigned char result[2];//比赛结果,比赛结束后需要更新，在规定的时间内更新，否则超过时间则需要对原始奖池进行分配，暂定平分
     double odds[2];//赔率.项目方设定的赔率
     std::string betting_end_time; //投注结束时间
     std::string start_time;//预计比赛开始时间
     std::string end_time;//预计比赛结束时间
     asset original_prize;//原始奖池，需要项目方存入相应的资金，暂定只能为EOS。由竞猜创造者转入。
     unsigned char freeze_flags;//冻结标志。一旦冻结，则不能进行任何操作
     unsigned char lottery_flags;//开奖标志。
     uint64_t primary_key() const { return id;}
     uint64_t get_user_name() const {return user.value;}
  };

  TABLE b_records{
      uint64_t bet_id;//投注ID
      uint64_t bet_id;//投注ID
      name user;//投注，直接原路返回到原转出地址
      std::string record_time;
      std::string player;//投注选手
      asset amount;//投注金额
      uint64_t primary_key() const {return bet_id;}
      uint64_t get_user_name() const {return user.value;}
  };

  TABLE b_prizePool{
     uint64_t id;
     asset playerA;//用来记录A方的总奖金池
     asset playerB;//用来记录B方的总奖金池
     asset original_prize;//初始奖池
     uint64_t primary_key() const {return id;}
  };

  TABLE f_games{

     uint64_t id;//投注ID，需要从服务器获取，保证id唯一
     std::vector<std::string>players;//对战双方
     std::vector<char>result;//比赛结果,比赛结束后需要更新，在规定的时间内更新，否则超过时间则需要对原始奖池进行分配，暂定平分
     std::string betting_end_time; //投注结束时间
     std::string start_time;//比赛开始时间
     std::string end_time;//预计比赛结束时间
     uint64_t primary_key() const { return id;}
  };

  TABLE g_games{
     //高尔夫的比赛规则暂定
     uint64_t id;
     std::vector<std::string>players;
     uint64_t primary_key() const { return id;}
 };

  TABLE t_game{
     //网球的比赛规则暂定
     uint64_t id;
     std::vector<std::string>players;
     uint64_t primary_key() const { return id;}
};

  typedef eosio::multi_index<
      "basket"_n, b_games,
       indexed_by<"byuser"_n, const_mem_fun<b_games, uint64_t,
                                            &b_games::get_user_name>>>
      basket_index;

  typedef eosio::multi_index<"basketrecord"_n, b_records>basket_record_index;
  typedef eosio::multi_index< "basketpool"_n,b_prizePool>basket_pool_index;

  basket_index bIndex;
  basket_pool_index bPool;
};

