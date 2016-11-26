#ifndef networking_h
#define networking_h

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

#include <iostream>
using std::cout;
using std::endl;
using std::cerr;

#include <string>
using std::string;

#include <sstream>
using std::istringstream;
using std::ostringstream;

#include <stdexcept>
using std::runtime_error;

#include <cryptopp/queue.h>
using CryptoPP::ByteQueue;

#include "cryptopp/cryptlib.h"
using CryptoPP::Exception;
using CryptoPP::PrivateKey;
using CryptoPP::PublicKey;
using CryptoPP::BufferedTransformation;
using CryptoPP::INFINITE_TIME;

#include "cryptopp/socketft.h"
using CryptoPP::Socket;
using CryptoPP::SocketSink;
using CryptoPP::SocketSource;

#include "cryptopp/rsa.h"
using CryptoPP::RSA;
using CryptoPP::RSASS;
using CryptoPP::InvertibleRSAFunction;

#include "cryptopp/pssr.h"
using CryptoPP::PSS;

#include "cryptopp/sha.h"
using CryptoPP::SHA256;

#include "cryptopp/files.h"
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include "cryptopp/filters.h"
using CryptoPP::ArraySink;
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::StringStore;
using CryptoPP::Redirector;
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;

#include "cryptopp/integer.h"
using CryptoPP::Integer;

#include "cryptopp/osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "cryptopp/secblock.h"
using CryptoPP::SecByteBlock;

#include "cryptopp/wait.h"
using CryptoPP::WaitObjectContainer;
using CryptoPP::CallStack;

const int MAX_CONN = 25;   			// Maximum number of socket connection the server will
                           			// 	backlog.
const int MAX_BUF = 500;

struct buddy
{
   struct sockaddr_in netInfo;   // Buddy connection information
   int sockDesc;                 // Buddy socket descriptor
   bool available;               // Flag for availability of the buddy

};

struct clientThreadData
{
   int tid;          // Thread ID.
   int sockDesc;   	// Client socket descriptor.
	string signature;
	StringSink * sig;
	AutoSeededRandomPool rng;
   RSA::PrivateKey privateKey;
   RSA::PublicKey publicKey;
	SocketSource sockListen;
	SocketSink sockSource;
};

int connectToHost(const char *, int);
int validatePort( const char * );
char * readFromSocket(int, char * &);
bool createSocket ( int & );
bool bindSocket ( int & , struct sockaddr_in &  );
bool listenSocket ( int & );
bool acceptSocket ( int &, int &, struct sockaddr_in &, socklen_t & );
void SavePrivateKey(const string& filename, const PrivateKey& key);
void SavePublicKey(const string& filename, const PublicKey& key);
void Save(const string& filename, const BufferedTransformation& bt);
void LoadPrivateKey(const string& filename, PrivateKey& key);
void LoadPublicKey(const string& filename, PublicKey& key);
void Load(const string& filename, BufferedTransformation& bt);
void sendMsg(Socket&, string, struct clientThreadData * );
string recoverMsg(Socket&,  struct clientThreadData * );
void sendMsg(RSA::PublicKey, Socket&, string);
string recoverMsg(RSA::PublicKey, Socket&);

#endif
