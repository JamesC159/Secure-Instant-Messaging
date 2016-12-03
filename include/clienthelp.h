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

#include <sys/socket.h>

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PublicKey;
using CryptoPP::BufferedTransformation;

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

#include <cryptopp/hex.h>
using CryptoPP::HexEncoder;

#include <cryptopp/filters.h>
using CryptoPP::StringSource;
using CryptoPP::StringSink;
const int MAX_BUF = 500;

void
sendMsg( RSA::PublicKey, Socket&, string );

string
recoverMsg( RSA::PublicKey, Socket& );

#endif /* CLIENTHELP_H_ */
