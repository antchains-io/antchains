
#include <eosiolib/eosio.hpp>
//#include <fc/crypto/sha256.hpp>
#include <eosiolib/crypto.h>
using namespace eosio;

CONTRACT uidnewacc : public eosio::contract {

public:
   using contract::contract;

   enum SUBJECTYPE { COMMENT, ROADBOOK, AIRTICKETS, HOTEL, OTHER };

   uidnewacc(name receiver, name code, datastream<const char *> ds)
         : contract(receiver, code, ds),
         uidnewaccIndex(_self, _self.value) {}

   ACTION insertuid(name user, const std::string& uid);
   ACTION newacc(name user, const std::string& publickey, const std::string& uid);
   ACTION eraseuid(const std::string& uid);
   
   using insertuid_action = action_wrapper<"insertuid"_n, &uidnewacc::insertuid>;
   using newacc_action = action_wrapper<"newacc"_n, &uidnewacc::newacc>;
   using eraseuid_action = action_wrapper<"eraseuid"_n, &uidnewacc::eraseuid>;

private:
   struct signup_public_key {
      uint8_t type;
      std::array<unsigned char, 33> data;
  };
  struct permission_level_weight {
      permission_level permission;
      uint16_t weight;
  };
  struct key_weight {
      signup_public_key key;
      uint16_t weight;
  };
  struct wait_weight {
      uint32_t wait_sec;
      uint16_t weight;
  };
  struct authority {
      uint32_t threshold;
      std::vector<key_weight> keys;
      std::vector<permission_level_weight> accounts;
      std::vector<wait_weight> waits;
  };
  struct newaccount {
      name creator;
      name name;
      authority owner;
      authority active;
  };

   TABLE uidnewaccs {
      name user;
      checksum256 sha256uid;
     
      uint64_t primary_key() const { return user.value; }
      key256 get_uid() const  { return get_uid(sha256uid); }

      static key256 get_uid(const checksum256 &sha256uid)
       {
           const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&sha256uid);
           return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
       }
   };

   typedef eosio::multi_index<
      "uidnewaccs"_n, uidnewaccs,
      indexed_by<"byuid"_n, const_mem_fun<uidnewaccs, key256,
                                             &uidnewaccs::get_uid>>>
      uidnewaccs_index;

   uidnewaccs_index uidnewaccIndex;
};

// Copied from https://github.com/bitcoin/bitcoin

/** All alphanumeric characters except for "0", "I", "O", and "l" */
static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const int8_t mapBase58[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
        -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
        22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
        -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
        47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
};

bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch)
{
    // Skip leading spaces.
    while (*psz && isspace(*psz))
        psz++;
    // Skip and count leading '1's.
    int zeroes = 0;
    int length = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    int size = strlen(psz) * 733 /1000 + 1; // log(58) / log(256), rounded up.
    std::vector<unsigned char> b256(size);
    // Process the characters.
    static_assert(sizeof(mapBase58)/sizeof(mapBase58[0]) == 256, "mapBase58.size() should be 256"); // guarantee not out of range
    while (*psz && !isspace(*psz)) {
        // Decode base58 character
        int carry = mapBase58[(uint8_t)*psz];
        if (carry == -1)  // Invalid b58 character
            return false;
        int i = 0;
        for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        length = i;
        psz++;
    }
    // Skip trailing spaces.
    while (isspace(*psz))
        psz++;
    if (*psz != 0)
        return false;
    // Skip leading zeroes in b256.
    std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
    while (it != b256.end() && *it == 0)
        it++;
    // Copy result into output vector.
    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end())
        vch.push_back(*(it++));
    return true;
}

bool decode_base58(const std::string& str, std::vector<unsigned char>& vch) {
    return DecodeBase58(str.c_str(), vch);
}
