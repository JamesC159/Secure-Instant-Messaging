#ifndef CLIENTDB_H_
#define CLIENTDB_H_

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <tuple>
using std::tuple;
using std::make_tuple;
using std::get;

#include <fstream>
using std::ifstream;

#include <string>
using std::string;

#include <iostream>

class ClientDB
{
private:
   tuple< string, string, string > clients[ 6 ];

public:
   ClientDB()
   {
   }

   ~ClientDB(){}

   inline void InitClients( ifstream& file )
   {
	  string username, digest, salt;

	  for ( int i = 0; i < 6; i++ )
	  {
		 file >> username >> digest >> salt;
		 clients[ i ] = make_tuple(username, digest, salt);
	  }
   }

   inline Integer FindUser( string username )
   {
	  for ( int i = 0; i < 6; i++ )
	  {
		 if ( username == get< 0 >(clients[ i ]))
		 {
			return Integer(get< 2 >(clients[ i ]).c_str());
		 }
	  }
	  return -1;
   }

   inline bool MatchDigest( string username, string digest )
   {
	  for ( int i = 0; i < 6; i++ )
	  {
		 if ( username == get< 0 >(clients[ i ]))
		 {
			if ( digest == get< 1 >(clients[ i ]))
			{
			   return true;
			}
			else
			{
			   return false;
			}
		 }
	  }
	  return false;
   }
};
#endif
