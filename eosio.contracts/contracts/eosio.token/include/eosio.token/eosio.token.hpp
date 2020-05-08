/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

#include <string>

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   class [[eosio::contract("eosio.token")]] token : public contract {
      public:
         using contract::contract;

         [[eosio::action]]
         void create( name   issuer,
                      asset  maximum_supply);

         [[eosio::action]]
         void issue( name to, asset quantity, string memo );

         [[eosio::action]]
         void retire( asset quantity, string memo );

         [[eosio::action]]
         void transfer( name    from,
                        name    to,
                        asset   quantity,
                        string  memo );

         [[eosio::action]]
         void open( name owner, const symbol& symbol, name ram_payer );

         [[eosio::action]]
         void close( name owner, const symbol& symbol );
         
         [[eosio::action]]
         void evo2eip( name    from,
                        name    to,
                        asset   quantity,
                        string  memo );

         [[eosio::action]]
         void eip2evo( name    from,
                        name    to,
                        asset   quantity,
                        string  memo );

         [[eosio::action]]
         void upserteval( symbol_code token,
                          uint64_t value,
                          uint64_t user_volume,
                          uint64_t order_volume,
                          uint64_t trans_volume);
                        
         static asset get_supply( name token_contract_account, symbol_code sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         static asset get_balance( name token_contract_account, name owner, symbol_code sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }

         using create_action = eosio::action_wrapper<"create"_n, &token::create>;
         using issue_action = eosio::action_wrapper<"issue"_n, &token::issue>;
         using retire_action = eosio::action_wrapper<"retire"_n, &token::retire>;
         using transfer_action = eosio::action_wrapper<"transfer"_n, &token::transfer>;
         using open_action = eosio::action_wrapper<"open"_n, &token::open>;
         using close_action = eosio::action_wrapper<"close"_n, &token::close>;
      private:
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         struct [[eosio::table]] token_evals {
            symbol_code token;
            uint64_t value;
            uint64_t user_volume;
            uint64_t order_volume;
            uint64_t trans_volume;

            uint64_t primary_key() const { return token.raw(); }
            uint64_t get_value() const { return value; }
            uint64_t get_user_volume() const { return user_volume; }
            uint64_t get_order_volume() const { return order_volume; }
            uint64_t get_trans_volume() const { return trans_volume; }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;
         typedef eosio::multi_index<"tokeneval"_n, token_evals,
                        indexed_by<"byvalue"_n, const_mem_fun<token_evals, uint64_t, &token_evals::get_value>>,
                        indexed_by<"byuser"_n, const_mem_fun<token_evals, uint64_t, &token_evals::get_user_volume>>,
                        indexed_by<"byorder"_n, const_mem_fun<token_evals, uint64_t, &token_evals::get_user_volume>>,
                        indexed_by<"bytrans"_n, const_mem_fun<token_evals, uint64_t, &token_evals::get_user_volume>>
                        > evals;
         void sub_balance( name owner, asset value );
         void add_balance( name owner, asset value, name ram_payer );
   };

} /// namespace eosio
