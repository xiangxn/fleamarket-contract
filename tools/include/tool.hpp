#pragma once

#include <string>
#include <math.h>
#include <errno.h>
#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include "base58.hpp"

using namespace std;
using namespace eosio;

namespace rareteam {

   bool is_zero(const checksum256& hash) {
      auto hash_data = hash.extract_as_byte_array();
      const uint64_t *p64 = reinterpret_cast<const uint64_t*>( hash_data.data() );
      return p64[0] == 0 && p64[1] == 0 && p64[2] == 0 && p64[3] == 0;
   }

   uint64_t get_uint64( const checksum256 hash, int32_t index = 0 ) {
      auto hash_data = hash.extract_as_byte_array();
      const uint64_t *p64 = reinterpret_cast<const uint64_t*>( hash_data.data() );
      if(index > 3 ) index = 3;
      return p64[index];
   }

   void set_zero(const checksum256& a )  {
      memset( (void *)a.data(), 0, 32 );
   }

   bool is_equal(const checksum256& a, const checksum256& b) {
      return memcmp((void *)a.data(), (const void *)b.data(), a.size()) == 0;
   }

   vector<string> split(const string& str, const string& delim) {  
      vector<string> res;  
      if("" == str) return res;  
      char * strs = new char[str.length() + 1] ;  
      strcpy(strs, str.c_str());   

      char * d = new char[delim.length() + 1];  
      strcpy(d, delim.c_str());  

      char *p = strtok(strs, d);  
      while(p) {  
         string s = p; 
         res.push_back(s);
         p = strtok(NULL, d);  
      }   
      return res;  
   }

   uint8_t from_hex(char c) {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'a' && c <= 'f') return c - 'a' + 10;
      if (c >= 'A' && c <= 'F') return c - 'A' + 10;
      check(false, "Invalid hex character");
      return 0;
   }

   size_t from_hex(const std::string& hex_str, char* out_data, size_t out_data_len) {
      auto i = hex_str.begin();
      uint8_t* out_pos = (uint8_t*)out_data;
      uint8_t* out_end = out_pos + out_data_len;
      while (i != hex_str.end() && out_end != out_pos) {
         *out_pos = from_hex((char)(*i)) << 4;
         ++i;
         if (i != hex_str.end()) {
               *out_pos |= from_hex((char)(*i));
               ++i;
         }
         ++out_pos;
      }
      return out_pos - (uint8_t*)out_data;
   }

   checksum256 hex_to_sha256(const std::string& hex_str) {
      check(hex_str.length() == 64, "invalid sha256");
      checksum256 checksum;
      from_hex(hex_str, (char*)checksum.data(), 32);
      return checksum;
   }

   string to_hex(uint8_t const * input, size_t length) {
      std::string output;
      std::string hex = "0123456789ABCDEF";

      for (size_t i = 0; i < length; i++) {
         output += hex[(input[i] & 0xF0) >> 4];
         output += hex[input[i] & 0x0F];
         //output += " ";
      }

      return output;
   }

   string to_hex(const char* input,size_t length) {
      return to_hex(reinterpret_cast<const uint8_t*>(input),length);
   }

   string sha256_to_hex(const checksum256& hash )
   {
      return to_hex( (char*)hash.data(), 32 );
   }

   uint64_t get_send_id( uint128_t sender_id )
   {
      return uint64_t( sender_id >> 64 );
   }

   string uint128ToString(const uint128_t& value)
   {
      const char* charmap = "0123456789";
      std::string result;
      result.reserve( 40 ); // max. 40 digits possible ( uint64_t has 20) 
      uint128_t helper = value;

      do {
         result += charmap[ helper % 10 ];
         helper /= 10;
      } while ( helper );
      std::reverse( result.begin(), result.end() );
      return result;
   }

   uint128_t uint128FromString(const string& sz)
   {
      uint128_t val = 0;
      if(!sz.empty()) {
        int radix = 10;
        bool minus = false;
        std::string::const_iterator i = sz.begin();
        if(*i == '-') {
          ++i;
          minus = true;
        }
        if(i != sz.end()) {
          if(*i == '0') {
            radix = 8;
            ++i;
            if(i != sz.end()) {
              if(*i == 'x') {
                radix = 16;
                ++i;
              }
            }
          }

          while(i != sz.end()) {
            unsigned int n = 0;
            const char ch = *i;

            if(ch >= 'A' && ch <= 'Z') {
              if(((ch - 'A') + 10) < radix) {
                n = (ch - 'A') + 10;
              } else {
                break;
              }
            } else if(ch >= 'a' && ch <= 'z') {
              if(((ch - 'a') + 10) < radix) {
                n = (ch - 'a') + 10;
              } else {
                break;
              }
            } else if(ch >= '0' && ch <= '9') {
              if((ch - '0') < radix) {
                n = (ch - '0');
              } else {
                break;
              }
            } else {
              break;
            }

            val *= radix;
            val += n;
            ++i;
          }
        }

        // if this was a negative number, do that two's compliment madness :-P
        if(minus) {
          val = -val;
        }
      }
      return val;
   }

   template<typename T>
   uint32_t get_size( T& table ) {
      uint32_t count = 0;
      auto itr = table.begin();
      while( itr != table.end() ) {
         count++;
         itr++;
      }
      return count;
   }

   template<typename T>
   void clear_table( T& table, uint32_t max = 500 ) {
      auto itr = table.begin();
      uint32_t cur = 0;
      while( itr != table.end() && cur < max ) {
         itr = table.erase( itr );
         cur++;
      }
   }

   bool IsChinese(const string& str) {
      bool chinese = true;
      for(int i = 0; i < str.length(); i++){
         if(str[i] >= 0){
            chinese = false;
            break;
         }
      }
      if(chinese) {
         return str.length() >= 6 && str.length() <= 24;
      }
      return chinese;
   }

   static bool IsWord(const string& str) {
      if(str.length() == 0) return false;
      char c = str.at(0);
      if(c >= '0' && c <= '9') return false;

      bool word = true;
      for(int i = 0; i < str.length(); i++) {
         c = str.at(i);
         if( !((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) ) {
            word = false;
            break;
         }
      }
      if(word) {
         return str.length() >= 3 && str.length() <= 12;
      }
      return word;
   }

   signature str_to_sig(const string& sig, const bool& checksumming = true) {
      const auto pivot = sig.find('_');
      check(pivot != string::npos, "No delimiter in signature");
      const auto prefix_str = sig.substr(0, pivot);
      check(prefix_str == "SIG", "Signature Key has invalid prefix");
      const auto next_pivot = sig.find('_', pivot + 1);
      check(next_pivot != string::npos, "No curve in signature");
      const auto curve = sig.substr(pivot + 1, next_pivot - pivot - 1);
      check(curve == "K1" || curve == "R1", "Incorrect curve");
      const bool k1 = curve == "K1";
      auto data_str = sig.substr(next_pivot + 1);
      check(!data_str.empty(), "Signature has no data");
      vector<unsigned char> vch;

      check(decode_base58(data_str, vch), "Decode signature failed");

      check(vch.size() == 69, "Invalid signature");

      if (checksumming) {
         array<unsigned char, 67> check_data;
         copy_n(vch.begin(), 65, check_data.begin());
         check_data[65] = k1 ? 'K' : 'R';
         check_data[66] = '1';

         checksum160 check_sig = ripemd160(reinterpret_cast<char*>(check_data.data()), 67);
         check(memcmp(check_sig.extract_as_byte_array().data(), &vch.end()[-4], 4) == 0, "Signature checksum mismatch");
      }

      signature _sig,_sig2;
      unsigned int type = k1 ? 0 : 1;
      _sig.type = (uint8_t)type;
      copy_n(vch.begin(), 65, _sig.data.begin());
      return _sig;
   }

   public_key str_to_pub(const string& pubkey, const string& pubkey_prefix = "EOS", const bool& checksumming = true) {
      auto base58substr = pubkey.substr(pubkey_prefix.length());
      vector<unsigned char> vch;
      check(decode_base58(base58substr, vch), "Decode public key failed");
      check(vch.size() == 37, "Invalid public key");
      if (checksumming) {
         array<unsigned char, 33> pubkey_data;
         copy_n(vch.begin(), 33, pubkey_data.begin());

         checksum160 check_pubkey = ripemd160(reinterpret_cast<char*>(pubkey_data.data()), 33);

         check(memcmp(check_pubkey.extract_as_byte_array().data(), &vch.end()[-4], 4) == 0, "Public key checksum mismatch");
      }
      public_key _pub_key;
      _pub_key.type = 0;
      copy_n(vch.begin(), 33, _pub_key.data.begin());
      return _pub_key;
   }

}