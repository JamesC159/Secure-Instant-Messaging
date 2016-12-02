/*
 * serverhelp.h
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#ifndef SERVERHELP_H_
#define SERVERHELP_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
using std::getline;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;
using std::istringstream;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PrivateKey;
using CryptoPP::INFINITE_TIME;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;
using CryptoPP::socklen_t;

#include <cryptopp/rsa.h>
using CryptoPP::RSA;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <cryptopp/sha.h>
using CryptoPP::SHA256;

const int MAX_BUF = 500;

struct ThreadData
{
	int tid; // Thread ID.
	RSA::PrivateKey privateKey;
	RSA::PublicKey publicKey;
	Socket sockListen;
	Socket sockSource;
	sockaddr_in clientaddr;
	socklen_t clientlen;
};

void
SendMsg( string, struct ThreadData * );

string
RecoverMsg( struct ThreadData * );

#endif /* SERVERHELP_H_ */
