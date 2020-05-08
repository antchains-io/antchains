#include "eosio.token.hpp"
#include <eosiolib/action.h>
#include <entrydetail.hpp>

using namespace eosio;

bool entry_type_is_valid(uint64_t type)
{
    return (type>=entrydetail::COMMENT&&type<=entrydetail::OTHER)?1:0;
}
ACTION entrydetail::entrycreate(name user, uint64_t entry_id,uint64_t entry_id_prior,std::vector<permission_level> permission,uint64_t type)
{
    bool account_exit_flag = false;
    eosio_assert(::entry_type_is_valid(type),"entry create type is err");
    eosio_assert(2==permission.size(),"entry create permission is too more");
    require_auth(user);
    std::vector<permission_level>::iterator iter;
    for(auto iter=permission.begin(); iter!=permission.end();++iter)
    {
       if(iter->actor==user)
       {
           break;
       }
    }
    eosio_assert(iter!=std::end(permission),"entry_create permission don't match");
    require_auth2(user.value,iter->permission.value);

    std::vector<std::pair<permission_level,uint8_t>> level1; 
    std::pair<permission_level,uint8_t> tmp;
    for(auto it=permission.begin(); it!=permission.end();++it)
    {
        eosio_assert(is_account(it->actor),"entry_create actor of permmision err");
        tmp.first.actor=it->actor ;
        tmp.first.permission=it->permission;
        tmp.second = 0x0;
        if(user==it->actor)
        {
            tmp.second = 0x1;
        }
        level1.push_back(tmp);
    }

    if(entry_id_prior)
    {
        auto iterator = entryIndex.find(entry_id_prior);
        eosio_assert(iterator != entryIndex.end(), "this entry is no exist");
    }

    auto iterator = entryIndex.find(entry_id);
    eosio_assert(iterator == entryIndex.end(), "this entry is exist.");

    entryIndex.emplace(_self,[&](auto &row) {
    row.user=user;
    row.type=type;
    row.entry_id=entry_id;
    row.entry_id_prior=entry_id_prior;
    row.entry_create_time=current_time();
    row.entry_effective_time=0x0;
    row.level=move(level1);
   });
}
ACTION entrydetail::entryeffect(uint64_t entry_id,permission_level permission,asset cash_back)
{
    eosio_assert(permission.actor==PRIVILEGE_ENTRY_ACCOUNT,"entry_effective permission err");

    auto iterator = entryIndex.find(entry_id);
    eosio_assert(iterator != entryIndex.end(), "this entry is no exist");
    
    std::vector<std::pair<permission_level,uint8_t>> level1;
    std::vector<std::pair<permission_level,uint8_t>>::iterator iter;
    //get the information that stores
    level1 = move(iterator->level);
    for(iter=level1.begin();iter!=level1.end();++iter)
    {
       if(iter->first.actor==PRIVILEGE_ENTRY_ACCOUNT)
       {
           break;
       }
    }
    eosio_assert(iter!=level1.end(),"entry_effective data err");
    eosio_assert(iter->second==0x0,"entry_effective has done");   
    iter->second = ENTRY_PERMISSION_EFFECTIVE;

    require_auth2(PRIVILEGE_ENTRY_ACCOUNT.value,iter->first.permission.value);

    entryIndex.modify(iterator, _self, [&](auto &row) {
        row.entry_effective_time = current_time();
        row.level= move(level1);
    });
    
    INLINE_ACTION_SENDER(eosio::token, transfer)(
         eosio::name("eosio.token"), {PRIVILEGE_ENTRY_ACCOUNT, eosio::name("active")},
         { PRIVILEGE_ENTRY_ACCOUNT, PRIVILEGE_ENTRY_ACCOUNT, cash_back, std::string("entry cash back") }
      );
    
}
ACTION entrydetail::entrycancle(uint64_t entry_id)
{
    auto iterator = entryIndex.find(entry_id);
    eosio_assert(iterator != entryIndex.end(), "this entry is no exist");
    std::vector<std::pair<permission_level,uint8_t>> level1;
    std::vector<std::pair<permission_level,uint8_t>>::iterator iter;
    //get the information that stores
    level1 = move(iterator->level);
    for(iter=level1.begin();iter!=level1.end();++iter)
    {
       if(iter->first.actor==PRIVILEGE_ENTRY_ACCOUNT)
       {
           break;
       }
    }
    
    eosio_assert(iter!=std::end(level1),"entry_effective data err");
    eosio_assert(iter->second==ENTRY_PERMISSION_INITIAL,"entry_effective has effective ,cannot cancle"); 

    require_auth2(PRIVILEGE_ENTRY_ACCOUNT.value,iter->first.permission.value);
    entryIndex.erase(iterator);
}

EOSIO_DISPATCH(entrydetail,(entrycreate)(entryeffect)(entrycancle))
