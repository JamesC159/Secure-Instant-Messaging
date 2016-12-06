#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <thread>
#include <mutex>
#include <sstream>
#include <iostream>
#include <string>

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



#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "getch.h"

extern std::string ownName;
extern std::string otherName;
extern Socket connectedSock;


void incomingRequestHandler();
void connReqHdlr(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket * sock);

void startTalking(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket * sock);
void sockListener(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket * sock);


// from client_main.cc
extern Socket incSock;

#endif
