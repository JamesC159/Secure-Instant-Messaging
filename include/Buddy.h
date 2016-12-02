#ifndef BUDDY_H_
#define BUDDY_H_

#include <sys/socket.h>

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;

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
	string username;
	string password;
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
		this->password = password;
		this->port = port;
	}
	Buddy( string username, string password, Integer port, Integer salt )
	{
		this->username = username;
		this->password = password;
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

	inline const
	string Getpassword()
	{
		return password;
	}

	inline const
	string Getusername()
	{
		return username;
	}

	inline void
	Setpassword( const string password )
	{
		this->password = password;
	}

	inline void
	Setusername( const string username )
	{
		this->username = username;
	}

	inline void
	SetSalt( const Integer salt )
	{
		this->salt = salt;
	}
};

#endif
