#include <lottery.hpp>
#include "eosio.token.hpp"

using namespace eosio;

static time_point_sec from_iso_string( const fc::string& s)
{
  static boost::posix_time::ptime epoch = boost::posix_time::from_time_t( 0 );
  boost::posix_time::ptime pt;
  if( s.size() >= 5 && s.at( 4 ) == '-' ) // http://en.wikipedia.org/wiki/ISO_8601
      pt = boost::date_time::parse_delimited_time<boost::posix_time::ptime>( s, 'T' );
  else
      pt = boost::posix_time::from_iso_string( s );
  return time_point_sec( (pt - epoch).total_seconds() );
}
ACTION lottery::bgenerate(uint64_t id,name user,std::vector<std::string>players, std::vector<double>odds,std::string start_time,
                                   std::string end_time,asset original_prize) {
//create game
require_auth(user);
auto itr = bIndex.find(id);
eosio_assert(itr==bIndex.end(),"the lottery games has exist");
auto itr_tmp= bPool.find(id);
eosio_assert(itr_tmp==bPool.end(),"the prize has exist");
eosio_assert(original_prize.is_valid(),"the aaset param error");
eosio_assert(original_prize>(original_prize-original_prize),"original_prize error");
eosio_assert(odds[0]>double(1),"odds param error");
eosio_assert(odds[1]>double(1),"odds param error");
eosio_assert(start_time<end_time,"time error");
//need to judge the time
// auto time1 = fc::time_point::now();
//EOS_ASSERT(time_point_sec( fc::time_point::from_iso_string(start_time) ) + 30) >= time_point_sec(time1), chain::expired_tx_exception,"the eip2evo transaction has expired");^M

//refresh the basket ball
bIndex.emplace(_self, [&](auto& p) {
        p.id  = id;
        p.user = user;
        p.players[0]=players[0];//string copy
        p.players[1]=players[1];//string copy
        p.odds[0]=odds[0];
        p.odds[1]=odds[1];
        p.start_time= start_time;
        p.end_time = end_time;
        p.original_prize = original_prize;
        p.freeze_flags = 0x0;
        p.lottery_flags = 0x0;
        p.betting_end_time = start_time;
    });
//refresh the pool prize
bPool.emplace(_self,[&](auto& p) {
        p.id  = id;
        p.playerA = asset(0,original_prize.symbol);
        p.playerB = asset(0,original_prize.symbol);
        p.original_prize = original_prize;

});
#if 1
action(
      permission_level{user,"active"_n},
      "eosio.token"_n,
      "transfer"_n,
      std::make_tuple(user, system_lottery_account, original_prize, std::string("create the basket lottery"))
    ).send();
#else
INLINE_ACTION_SENDER(eosio::token, transfer)(
         eosio::name("eosio.token"), {user, eosio::name("active")},
         { user, system_lottery_account, original_prize, std::string("create the basket lottery") });
#endif
}

ACTION lottery::bbet(uint64_t id ,uint64_t bet_id,name user,std::string player,std::string bet_time,asset amount) {
  //普通用户投注
  require_auth(user);
  auto itr = bIndex.find(id);
eosio_assert(itr!=bIndex.end(),"the lottery does not exist");
  auto flag = (player==itr->players[0])||(player==itr->players[1]);
  eosio_assert(flag,"the player fo bet does not exit");
  auto flag_playerA = (player==itr->players[0]);
  auto flag_playerB = (player==itr->players[1]);
  eosio_assert(!(flag_playerA&flag_playerB),"the param of game error");
  eosio_assert(amount.is_valid(),"the aaset param error");
  eosio_assert(amount>(amount-amount),"the amount of bnet error");
  eosio_assert(!itr->freeze_flags,"the game has freeze");
  //需要确认时间
  eosio_assert(bet_time<itr->start_time, "the game is now begining");
  //eosio_assert(time_point_sec( fc::time_point::from_iso_string(itr->start_time)) >current_time(), "the game is now begining");
  //根据比赛id来确定比赛
  basket_record_index bRecord(_self,id);
  auto itr_tmp =bRecord.find(bet_id);
  eosio_assert(itr_tmp==bRecord.end(),"the id of bet has exist");
  //refresh the bet record
  bRecord.emplace(_self,[&](auto& p) {
    p.bet_id  = bet_id;
    p.user = user;
    p.player = player;
    p.amount = amount;
    p.record_time=bet_time;
  });

  //refresh the prize pool
  auto itr1=bPool.find(id);
  eosio_assert(itr1!=bPool.end(),"the prize pool does not exist");
  bPool.modify(itr1, user, [&](auto& p) {
         if(flag_playerA)
         {
             eosio_assert((p.playerA+amount).is_valid(),"the bnet amount overflow");
             p.playerA+=amount;
         }
         else
         {
             eosio_assert((p.playerB+amount).is_valid(),"the bnet amount overflow");
             p.playerB+=amount;
         }
    });

#if 1
action(
      permission_level{user,"active"_n},
      "eosio.token"_n,
      "transfer"_n,
      std::make_tuple(user, system_lottery_account, amount, std::string("bnet the game"))
    ).send();
#else
    INLINE_ACTION_SENDER(eosio::token, transfer)(
         eosio::name("eosio.token"), {user, eosio::name("active")},
         { user, system_lottery_account, amount, std::string("bnet the game") });
#endif
}

ACTION lottery::bfreeze(uint64_t id,name user) {
  //冻结比赛
  require_auth(user);
  auto itr = bIndex.find(id);
  eosio_assert(itr!=bIndex.end(),"the lottery games does not exit");
  eosio_assert(!itr->freeze_flags,"the lottery games has freeze");
  eosio_assert(user==itr->user,"user cannot freeze the game");
//需要判断时间,冻结时间必须在比赛结束之前
  //eosio_assert(time_point_sec( fc::time_point::from_iso_string(itr->end_time)) >current_time(), "the game is now begining");
  bIndex.modify(itr, user, [&](auto& p) {
      p.freeze_flags = 0x1;
  });
}

ACTION lottery::brefund(uint64_t id ,uint64_t bnet_id,uint64_t user)
{
    //退款
    require_auth(user);
    auto itr=bIndex.find(id);
    eosio_assert(itr!=bIndex.end(),"the id of game does not exist");
    eosio_assert((itr->freeze_flags==0x2),"the bet game does not cancle");
    basket_record_index bRecord(_self,id);
    auto itr_tmp =bRecord.find(bnet_id);
    eosio_assert(itr_tmp!=bRecord.end(),"the id of bet does not exit");
    auto amount = itr_tmp->amount;
    auto to = itr_tmp->user;
    bRecord.erase(itr_tmp);

    #if 1
    action(
     permission_level{system_lottery_account,"active"_n},
      "eosio.token"_n,
      "transfer"_n,
      std::make_tuple(system_lottery_account, to, amount, std::string("refund bet because of games cancle"))
    ).send();
    #else
     INLINE_ACTION_SENDER(eosio::token, transfer)(
         eosio::name("eosio.token"), {system_lottery_account, eosio::name("active")},
         { system_lottery_account, user, amount, std::string("refund bet because of games cancle") });
    #endif
}

ACTION lottery::bcancle(uint64_t id)
{

  require_auth(system_lottery_account);
  auto itr=bIndex.find(id);
  eosio_assert(itr!=bIndex.end(),"the id of game does not exist");
  eosio_assert((itr->freeze_flags==0x1),"the id of game does not freeze");
  asset amount = itr->original_prize;
  auto create = itr->user;
  bIndex.modify(itr, system_lottery_account, [&](auto& p){
     p.freeze_flags = 2;
     p.original_prize =asset(0,amount.symbol) ;
  });
  #if 1
  action(
  permission_level{system_lottery_account,"active"_n},
      "eosio.token"_n,
      "transfer"_n,
      std::make_tuple(system_lottery_account, create, amount, std::string("refund the original_prize"))
    ).send();
  #else
  INLINE_ACTION_SENDER(eosio::token, transfer)(
         eosio::name("eosio.token"), {system_lottery_account, eosio::name("active")},
         { system_lottery_account, create, amount, std::string("refund the original_prize") });
  #endif
}

ACTION lottery::blottery(uint64_t id,name user,std::vector<unsigned char>final_result){

   //由运营方开奖，无条件信任
   require_auth(system_lottery_account);
   auto itr=bIndex.find(id);
   eosio_assert(itr!=bIndex.end(),"the id of game does not exit");
   eosio_assert(final_result[0]!=final_result[1],"final_result param error");
   bIndex.modify(itr, system_lottery_account, [&](auto& p){
   p.result[0] = final_result[0];
     p.result[1] = final_result[1];
     p.lottery_flags = 0x1;
  });
}

ACTION lottery::bredemption(uint64_t id,uint64_t bnet_id,name user){

    require_auth(user);
    auto itr = bIndex.find(id);
    eosio_assert(itr!=bIndex.end(),"the lottery game does not exist");
    eosio_assert(itr->lottery_flags,"the game has not lottery");
    auto win_flag = itr->result[0]>itr->result[1]?0:1;
    basket_record_index bRecord(_self,id);
    auto itr1= bRecord.find(bnet_id);
    eosio_assert(itr1!=bRecord.end(),"the lotter bnet id record does noet exist");
    auto amount = itr1->amount;
    auto player = itr1->player;
    auto to = itr1->user;
    eosio_assert((player!=itr->players[0])&&(player!=itr->players[1]),"players mot match the record");
    bRecord.erase(itr1);
    if(player==itr->players[0])
    {
       //
       if(win_flag)
       {
          //确保小数点只有两位
          auto amount1 = static_cast<int64_t>((amount.amount*(static_cast<int64_t>(itr->odds[0]*100)))/100);
          amount.amount= amount1;
       }
    }
    else
    {
        //
       if(!win_flag)
       {
          //确保小数点只有两位
         auto amount1 = static_cast<int64_t>((amount.amount*(static_cast<int64_t>(itr->odds[1]*100)))/100);
          amount.amount = amount1;
       }
    }
    eosio_assert(amount.is_valid(),"asset overflow");//check the amount if overflow

    #if 1
      action(
      permission_level{system_lottery_account,"active"_n},
      "eosio.token"_n,
      "transfer"_n,
      std::make_tuple(system_lottery_account, to, amount, std::string("redemption"))
    ).send();
    #else
    INLINE_ACTION_SENDER(eosio::token, transfer)(
         eosio::name("eosio.token"), {system_lottery_account, eosio::name("active")},
         { system_lottery_account, to, amount, std::string("redemption") });
    #endif
}
EOSIO_DISPATCH(lottery, (bgenerate)(bbet)(bfreeze)(bcancle)(blottery)(bredemption)(brefund))

