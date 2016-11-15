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


int connectToHost(const char *, int);
int validatePort( const char * );
char * readFromSocket(int, char * &);
bool createSocket ( int & );
bool bindSocket ( int & , struct sockaddr_in &  );
bool listenSocket ( int & );
bool acceptSocket ( int &, int &, struct sockaddr_in &, socklen_t & );
