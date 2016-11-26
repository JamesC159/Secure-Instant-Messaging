/*
 * clienthelp.h
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#ifndef CLIENTHELP_H_
#define CLIENTHELP_H_

#include <iostream>
using std::cout;
using std::endl;
using std::cerr;

#include <sys/time.h>

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PublicKey;
using CryptoPP::word32;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;

#include <cryptopp/rsa.h>
using CryptoPP::RSA;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <cryptopp/sha.h>
using CryptoPP::SHA256;

#include <cryptopp/filters.h>
using CryptoPP::ArraySink;
using CryptoPP::HashFilter;

const int MAX_BUF = 500;

void
sendMsg(RSA::PublicKey, Socket&, string);

string
recoverMsg(RSA::PublicKey, Socket&);

#endif /* CLIENTHELP_H_ */
