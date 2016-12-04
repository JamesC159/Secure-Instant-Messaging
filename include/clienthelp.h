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

#include <algorithm>
using std::min;

#include <sys/socket.h>

#include <string>
using std::string;

#include <sstream>
using std::stringstream;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PublicKey;
using CryptoPP::BufferedTransformation;
using CryptoPP::DecodingResult;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;

#include <cryptopp/pssr.h>
using CryptoPP::PSSR;

#include <cryptopp/rsa.h>
using CryptoPP::RSA;
using CryptoPP::RSASS;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <cryptopp/sha.h>
using CryptoPP::SHA256;

#include <cryptopp/hex.h>
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include <cryptopp/hmac.h>
using CryptoPP::HMAC;

#include <cryptopp/filters.h>
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::HashFilter;
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;

#include <cryptopp/nbtheory.h>
using CryptoPP::ModularExponentiation;

#include <cryptopp/dh.h>
using CryptoPP::DH;

#include <cryptopp/dh2.h>
using CryptoPP::DH2;

#include <cryptopp/secblock.h>
using CryptoPP::SecByteBlock;

#include <stdexcept>
using std::runtime_error;

const int MAX_BUF = 500;
extern RSA::PublicKey serverKey;

void
sendMsg( Socket&, string );

string
recoverMsg( Socket& );

int swrite(Socket * sock, const char * buff, int len);
int sread(Socket * sock, char * buff, int len);

#endif /* CLIENTHELP_H_ */
