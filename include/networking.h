#ifndef networking_h
#define networking.h

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

const int MAX_CONN = 25;   // Maximum number of socket connection the server will
                           // backlog
const char * FIN_STR = "FIN\n\0";
const char * SYN_STR = "SYN\n\0";
const char * RST_STR = "RST\n\0";

int connectToHost(const char *, int);
int validatePort( const char * );
char * readFromSocket(int, char * &);
bool createSocket ( int & );
bool bindSocket ( int & , struct sockaddr_in &  );
bool listenSocket ( int & );
bool acceptSocket ( int &, int &, struct sockaddr_in &, socklen_t & );
#endif