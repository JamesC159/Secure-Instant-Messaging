#include "Socket.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <string>

Socket::Socket() : sockDesc ( -1 ) {
   
	memset( & sockAddr,
			0,
			sizeof ( sockAddr ) );
}

Socket::~Socket() {
   
	if ( isOpen() ) {
		if ( close ( sockDesc ) == -1 ) {
         fprintf( stderr, "Failed to close socket" );
      }
	}
}

bool Socket::create() {
   
	sockDesc = socket( AF_INET, 
						SOCK_STREAM, 
						0 );
   
	if ( ! isOpen() ) {
		return false;
	}
   
	return true;
}

bool Socket::bind( const int port ) {
   
   if ( ! isOpen() ) {
      return false;
   }
   
   sockAddr.sin_family = AF_INET;
   sockAddr.sin_port = htons( port );
   sockAddr.sin_addr.s_addr = INADDR_ANY; // Will bind the server to all network
                                          // interfaces. s_addr = 0.0.0.0
   
   if ( ::bind ( sockDesc,
                ( struct sockaddr * ) & sockAddr,
                sizeof ( sockAddr ) ) == -1 ) {
		return false;
	}
   
	return true;
}

bool Socket::listen() const {
   
   if ( ! isOpen() ) {
      return false;
   }
   
   int result = ::listen ( sockDesc, MAX_CONN );
   if ( result == -1 ) {
      return false;
   }
   
   return true;
}

bool Socket::accept( Socket & newSock ) {
   
   // This could cause an issue with sockAddr being overwritten since it is used
   // to bind the server to an address and accept client connection requests.
   newSock.sockDesc = ::accept( sockDesc,
                               ( struct sockaddr * ) & sockAddr,
                               ( socklen_t * ) sizeof( sockAddr ) );
   
   if( newSock.sockDesc == -1 ) {
      return false;
   }
   
   return true;
}

bool Socket::connect( std::string hostName, const int portno ) {
   
   if ( ! isOpen() ) {
      return false;
   }
   
   sockAddr.sin_family = AF_INET;
   sockAddr.sin_port = htons( portno );
   // Note that inet_pton does not allow 1, 2, or 3 part dotted IP addresses
   // Only 4 part address are accepted.
   int ptonResult = inet_pton( AF_INET, hostName.c_str(), & sockAddr.sin_addr );
   
   if( ptonResult == 0 ) {
      fprintf( stderr,
              "Error: inet_pton() returned 0. The address %s was not parseable in"
              " the AF_INET address family.",
              hostName.c_str() );
      return false;
   }
   else if ( ptonResult == -1 ) {
      perror( "inet_pton() Error: " );
      return false;
   }
   
   if( ::connect( sockDesc,
                 ( struct sockaddr * ) & sockAddr,
                 ( socklen_t ) sizeof( sockAddr ) ) == -1 ) {
      perror( "Socket Connection Error: " );
      return false;
   }
   
   return true;
}
