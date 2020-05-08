
#include <registration.hpp>

ACTION registration::insert(name referrer, name user, const std::string& publickey, const std::string &identifier) {

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
        .creator = referrer,
        .name = user,
        .owner = owner,
        .active = active
    };

    action(
            permission_level{ referrer, "active"_n },
            "eosio"_n,
            "newaccount"_n,
            new_account
    ).send();

    registrations_index registrationsIndex(_self, _self.value);
    registrationsIndex.emplace(_self, [&](auto &row) {
        row.referrer = referrer;
        row.user = user;
        row.pubkey = publickey;
        row.identifier = identifier;
    });
}

EOSIO_DISPATCH( registration, (insert))