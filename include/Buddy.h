#ifndef BUDDY_H_
#define BUDDY_H_

#include <sys/socket.h>

#include "cryptopp/ccm.h"
using CryptoPP::CBC_Mode;

#include <cryptopp/aes.h>
using CryptoPP::AES;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;

#include "cryptopp/cmac.h"
using CryptoPP::CMAC;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <exception>
using std::invalid_argument;
using std::exception;

#include <string>
using std::string;

class Buddy : public exception
{

private:

   CBC_Mode< AES >::Encryption enc;
   CBC_Mode< AES >::Decryption dec;
   CMAC< AES > cmac;
   string username;
   Socket sock;
   Integer salt;
   Integer port;

public:
   Buddy()
   {
   }
   Buddy( string username, Integer port )
   {
	  this->username = username;
	  this->port = port;
   }
   Buddy( string username, string password, Integer port )
   {
	  this->username = username;
	  this->port = port;
   }
   Buddy( string username, string password, Integer port, Integer salt )
   {
	  this->username = username;
	  this->port = port;
	  this->salt = salt;
   }
   ~Buddy()
   {
	  sock.ShutDown( SHUT_RDWR);
   }

   virtual const char*
   what() const throw ()
   {
	  return "My exception happened";
   }

   inline const string Getusername()
   {
	  return username;
   }

   inline const CBC_Mode< AES >::Encryption GetEnc()
   {
	  return enc;
   }

   inline const CBC_Mode< AES >::Decryption GetDec()
   {
	  return dec;
   }

   inline const CMAC< AES > GetCMAC()
   {
	  return cmac;
   }

   inline void SetEnc( CBC_Mode< AES >::Encryption enc )
   {
	  this->enc = enc;
   }

   inline void SetDec( CBC_Mode< AES >::Decryption dec )
   {
	  this->dec = dec;
   }

   inline void SetCMAC( CMAC< AES > cmac )
   {
	  this->cmac = cmac;
   }

   inline void Setusername( const string username )
   {
	  this->username = username;
   }

   inline void SetSalt( const Integer salt )
   {
	  this->salt = salt;
   }
};

#endif
