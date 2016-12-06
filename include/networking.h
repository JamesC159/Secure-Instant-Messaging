#ifndef networking_h
#define networking_h

#include <string>
using std::string;

#include <cryptopp/cryptlib.h>
using CryptoPP::PrivateKey;
using CryptoPP::PublicKey;
using CryptoPP::BufferedTransformation;

#include <cryptopp/queue.h>
using CryptoPP::ByteQueue;

#include <cryptopp/files.h>
using CryptoPP::FileSource;
using CryptoPP::FileSink;

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

const int port = 15653;

void
SavePrivateKey( const string& filename, const PrivateKey& key );

void
SavePublicKey( const string& filename, const PublicKey& key );

void
Save( const string& filename, const BufferedTransformation& bt );

void
LoadPrivateKey( const string& filename, PrivateKey& key );

void
LoadPublicKey( const string& filename, PublicKey& key );

void
Load( const string& filename, BufferedTransformation& bt );

#endif
