#ifndef BUDDYLIST_H_
#define BUDDYLIST_H_

#include <Buddy.h>

#include <map>
using std::make_pair;

#include <unordered_map>
using std::unordered_map;

#include "cryptopp/cmac.h"
using CryptoPP::CMAC;

#include <string>
using std::string;

class BuddyList
{
private:
   unordered_map< string, Buddy* > buddies;


public:
   BuddyList()
   {
	  buddies.clear();
   }

   ~BuddyList()
   {
	  for ( auto& pair : buddies )
	  {
		 delete pair.second;
	  }
	  buddies.clear();
   }

   inline void AddBuddy( string username, Integer port )
   {
	  buddies.insert(
		       make_pair< string&, Buddy* >(username,
		                new Buddy(username, port)));
   }

   inline Buddy*
   FindBuddy( string username )
   {
	  auto it = buddies.find(username);
	  if ( it == buddies.end() )
	  {
		 return nullptr;
	  }
	  else
	  {
		 return it->second;
	  }
   }

};

#endif
