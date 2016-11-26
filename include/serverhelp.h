/*
 * serverhelp.h
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#ifndef SERVERHELP_H_
#define SERVERHELP_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

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
using std::stringstream;
using std::ostringstream;
using std::istringstream;

#include <cryptopp/cryptlib.h>
using CryptoPP::Exception;
using CryptoPP::PrivateKey;
using CryptoPP::INFINITE_TIME;
using CryptoPP::word32;

#include <cryptopp/socketft.h>
using CryptoPP::Socket;
using CryptoPP::SocketSource;
using CryptoPP::SocketSink;
using CryptoPP::socklen_t;

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

#include <cryptopp/sha.h>
using CryptoPP::SHA256;

const int MAX_BUF = 500;


struct ThreadData
{
  int tid; // Thread ID.
  RSA::PrivateKey privateKey;
  RSA::PublicKey publicKey;
  Socket sockListen;
  Socket sockSource;
  sockaddr_in clientaddr;
  socklen_t clientlen;
};

void
sendMsg(string, struct ThreadData *);

string
recoverMsg(struct ThreadData *);

#endif /* SERVERHELP_H_ */
