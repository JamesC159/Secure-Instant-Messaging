/*
 * serverhelp.h
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#ifndef SERVERHELP_H_
#define SERVERHELP_H_

#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::cerr;
using std::flush;

#include <sys/time.h>

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PrivateKey;
using CryptoPP::INFINITE_TIME;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;
using CryptoPP::SocketSource;
using CryptoPP::SocketSink;

#include <cryptopp/rsa.h>
using CryptoPP::RSA;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <cryptopp/wait.h>
using CryptoPP::CallStack;
using CryptoPP::WaitObjectContainer;

#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::StringSinkTemplate;

const int MAX_BUF = 500;


struct clientThreadData
{
  int tid; // Thread ID.
  RSA::PrivateKey privateKey;
  RSA::PublicKey publicKey;
};

void
sendMsg(Socket&, string, struct clientThreadData *);

string
recoverMsg(Socket&, struct clientThreadData *);


#endif /* SERVERHELP_H_ */
