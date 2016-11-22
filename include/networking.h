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

#include <errno.h>

/*#include "rsa.h"
using CryptoPP::RSA;

#include "osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include "hrtimer.h"
using CryptoPP::TimerBase;
using CryptoPP::ThreadUserTimer;*/

const int MAX_CONN = 25;   			// Maximum number of socket connection the server will
                           			// 	backlog.

struct buddy
{
   struct sockaddr_in netInfo;   // Buddy connection information
   int sockDesc;                 // Buddy socket descriptor
   bool available;               // Flag for availability of the buddy

};

int connectToHost(const char *, int);
int validatePort( const char * );
char * readFromSocket(int, char * &);
bool createSocket ( int & );
bool bindSocket ( int & , struct sockaddr_in &  );
bool listenSocket ( int & );
bool acceptSocket ( int &, int &, struct sockaddr_in &, socklen_t & );

#endif