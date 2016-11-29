#ifndef BUDDY_H_
#define BUDDY_H_

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::word32;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <exception>
using std::invalid_argument;
using std::exception;

#include <string>
using std::string;

class Buddy: public exception
{

private:
	string uname;
	string pw;
	Socket sock;
	Integer salt;
	bool available;

public:
	Buddy()
	{
	}
	Buddy(string uname, string pw)
	{
		this->uname = uname;
		this->pw = pw;
		available = false;
	}
	~Buddy()
	{
	}

	virtual const char*
	what() const throw ()
	{
		return "My exception happened";
	}

	const string GetPW()
	{
		return pw;
	}

	const string GetUname()
	{
		return uname;
	}

	const bool GetAvailable()
	{
		return available;
	}

	const Integer GetSalt()
	{
		return salt;
	}

	void SetPW(const string pw)
	{
		this->pw = pw;
	}

	void SetUname(const string uname)
	{
		this->uname = uname;
	}

	void SetAvailable(const bool a)
	{
		available = a;
	}

	void SetSalt(const Integer salt)
	{
		this->salt = salt;
	}
};

#endif
