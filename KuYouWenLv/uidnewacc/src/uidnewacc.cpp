#include <uidnewacc.hpp>

ACTION uidnewacc::insertuid(name user, const std::string& uid) {
   require_auth(_self);
   checksum256 luid = sha256(uid.data(),uid.length());

   auto iterator = uidnewaccIndex.find(user.value);
   eosio_assert(iterator == uidnewaccIndex.end(), "this user exist.");
   
  auto ipfskeyidx =uidnewaccIndex.get_index<"byuid"_n>();
  auto itr2 = ipfskeyidx.find(uidnewaccs::get_uid(luid));
  //如果判断条件不成立，则终止执行并打印错误信息
  eosio_assert(itr2 == ipfskeyidx.end(), "ipfs sha256 for account already exists");
   
   uidnewaccIndex.emplace(_self, [&](auto &row) {
      row.user = user;
      row.sha256uid= luid;
   });
}

ACTION uidnewacc::newacc(name user, const std::string& publickey, const std::string& uid) {

   checksum256 luid = sha256(uid.data(),uid.length());

   auto iterator = uidnewaccIndex.find(user.value);
   eosio_assert(iterator!= uidnewaccIndex.end(), "this user do not exist in data base.");
   
   auto ipfskeyidx =uidnewaccIndex.get_index<"byuid"_n>();
   auto itr2 = ipfskeyidx.find(uidnewaccs::get_uid(luid));
   
   eosio_assert(itr2!= ipfskeyidx.end(), "this uid do not exist in data base.");

   eosio_assert(publickey.length() == 53, "Length of public key should be 53");

   std::string pubkey_prefix("EVS");
   auto result = mismatch(pubkey_prefix.begin(), pubkey_prefix.end(), publickey.begin());
   eosio_assert(result.first == pubkey_prefix.end(), "Public key should be prefix with EVS");
   auto base58substr = publickey.substr(pubkey_prefix.length());

   std::vector<unsigned char> vch;
   eosio_assert(decode_base58(base58substr, vch), "Decode pubkey failed");
   eosio_assert(vch.size() == 37, "Invalid public key");

   std::array<unsigned char,33> pubkey_data;
   copy_n(vch.begin(), 33, pubkey_data.begin());

   capi_checksum160 check_pubkey;
   ripemd160(reinterpret_cast<char *>(pubkey_data.data()), 33, &check_pubkey);
   eosio_assert(memcmp(&check_pubkey.hash, &vch.end()[-4], 4) == 0, "invalid public key");

   signup_public_key pubkey = {
      .type = 0,
      .data = pubkey_data,
   };
   key_weight pubkey_weight = {
      .key = pubkey,
      .weight = 1,
   };
   authority owner = authority{
      .threshold = 1,
      .keys = {pubkey_weight},
      .accounts = {},
      .waits = {}
   };
   authority active = authority{
      .threshold = 1,
      .keys = {pubkey_weight},
      .accounts = {},
      .waits = {}
   };
   newaccount new_account = newaccount{
      .creator = "evscreator"_n,
      .name = user,
      .owner = owner,
      .active = active
   };

   action(
         permission_level{ "evscreator"_n, "active"_n },
         "eosio"_n,
         "newaccount"_n,
         new_account
   ).send();

   uidnewaccIndex.erase(iterator);
   //auto iterator_after = uidnewaccIndex.find( luid );
   //eosio_assert(iterator_after == uidnewaccIndex.end(),
   //            "uid erase failed after new account");
}

ACTION uidnewacc::eraseuid(const std::string& uid) {
  /*require_auth(_self);
  checksum256 luid = sha256(uid.data(),uid.length());
  auto iterator = uidnewaccIndex.find(luid);
  eosio_assert(iterator != uidnewaccIndex.end(),
               "uid record does not exist");
  uidnewaccIndex.erase(iterator);*/
}

EOSIO_DISPATCH(uidnewacc, (insertuid)(newacc)(eraseuid))
