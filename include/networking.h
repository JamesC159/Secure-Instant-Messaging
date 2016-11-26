#ifndef networking_h
#define networking_h

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>

#include <iostream>
using std::cout;
using std::endl;
using std::cerr;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include <stdexcept>
using std::runtime_error;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PrivateKey;
using CryptoPP::PublicKey;
using CryptoPP::BufferedTransformation;

#include <cryptopp/queue.h>
using CryptoPP::ByteQueue;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;

#include <cryptopp/files.h>
using CryptoPP::FileSource;
using CryptoPP::FileSink;

#include <cryptopp/rsa.h>
using CryptoPP::RSA;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

const int port = 5010;

void
SavePrivateKey(const string& filename, const PrivateKey& key);

void
SavePublicKey(const string& filename, const PublicKey& key);

void
Save(const string& filename, const BufferedTransformation& bt);

void
LoadPrivateKey(const string& filename, PrivateKey& key);

void
LoadPublicKey(const string& filename, PublicKey& key);

void
Load(const string& filename, BufferedTransformation& bt);

#endif
