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
using CryptoPP::AuthenticatedSymmetricCipher;

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

#include <cryptopp/base64.h>
using CryptoPP::Base64Encoder;
using CryptoPP::Base64Decoder;

#include <cryptopp/hmac.h>
using CryptoPP::HMAC;

#include "cryptopp/ccm.h"
using CryptoPP::CBC_Mode;

#include <cryptopp/aes.h>
using CryptoPP::AES;

#include <cryptopp/filters.h>
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::ArraySource;
using CryptoPP::ArraySink;
using CryptoPP::HashFilter;
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;
using CryptoPP::AuthenticatedEncryptionFilter;
using CryptoPP::AuthenticatedDecryptionFilter;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::HashVerificationFilter;

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

#include "cryptopp/cmac.h"
using CryptoPP::CMAC;

#include "assert.h"

const int MAX_BUF = 500;
const string FIN_STR = "FIN"; // These flags can be whatever we want them to be.
const string SYN_STR = "SYN";
const string RST_STR = "RST";
extern RSA::PublicKey serverKey;

void
sendMsg( Socket&, string );

string
recoverMsg( Socket& );

int symWrite(CBC_Mode< AES >::Encryption, CMAC< AES >, Socket * sock, const char * buff, int len);
int symRead(CBC_Mode< AES >::Decryption, CMAC< AES >, Socket * sock, char * buff, int len);
void CheckFin( string );


#endif /* CLIENTHELP_H_ */
