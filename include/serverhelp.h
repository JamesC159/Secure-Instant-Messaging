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
using std::stringstream;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PrivateKey;
using CryptoPP::INFINITE_TIME;
using CryptoPP::DecodingResult;
using CryptoPP::AuthenticatedSymmetricCipher;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;
using CryptoPP::socklen_t;

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

#include "cryptopp/ccm.h"
using CryptoPP::CBC_Mode;

#include <cryptopp/aes.h>
using CryptoPP::AES;

#include <cryptopp/hmac.h>
using CryptoPP::HMAC;

#include "cryptopp/cmac.h"
using CryptoPP::CMAC;

#include <cryptopp/filters.h>
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::HashFilter;
using CryptoPP::HashVerificationFilter;
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;
using CryptoPP::ArraySource;
using CryptoPP::ArraySink;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;
using CryptoPP::StreamTransformationFilter;

#include <cryptopp/nbtheory.h>
using CryptoPP::ModularExponentiation;

#include <cryptopp/dh.h>
using CryptoPP::DH;

#include <cryptopp/dh2.h>
using CryptoPP::DH2;

#include <cryptopp/secblock.h>
using CryptoPP::SecByteBlock;

#include <tuple>
using std::tuple;
using std::make_tuple;
using std::get;

#include <stdexcept>
using std::runtime_error;

#include "assert.h"

const int MAX_BUF = 500;

struct ThreadData
{
   int tid; // Thread ID.
   RSASS<PSSR, SHA256>::Signer signer;
   RSA::PrivateKey privateKey;
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
