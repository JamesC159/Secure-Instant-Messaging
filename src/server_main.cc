#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include <iostream>
#include <string>

const int MAX_BUF = 8192;
const int MAX_CONN = 25;

bool
createSocket ( int * sockDesc ) {
   
   *sockDesc = socket ( AF_INET,
                       SOCK_STREAM,
                       0 );
   
   if ( *sockDesc < 0 ) {
      return false;
   }
   
   return true;
}

bool
bindSocket ( int * sockDesc,
                 struct sockaddr_in * sockAddr ) {
   
   if ( bind( *sockDesc,
             (struct sockaddr *)sockAddr,
             (socklen_t)sizeof( *sockAddr )) < 0 ) {
      
      return false;
   }
   
   return true;
}

bool
listenSocket ( int * sockDesc ) {
   
   if ( listen ( *sockDesc, MAX_CONN ) < 0 ) {
      return false;
   }
   
   return true;
}

bool
acceptSocket ( int * servDesc,
                   int * cliDesc,
                   struct sockaddr_in * cliAddr ) {
   
   *cliDesc = accept( *servDesc,
                    (struct sockaddr *)cliAddr,
                    (socklen_t *)sizeof( *cliAddr ) );
   
   if ( *cliDesc < 0 ) {
      return false;
   }
   
   return true;
}

// main()
int
main ( int argc, char ** argv ) {
   
   struct sockaddr_in serverAddr, clientAddr;
   int serverDesc = -1, clientDesc = -1;
   int bytes = 0, portNo = -1;
   char buffer [ MAX_BUF ];
   char * endptr;
   
   // Make sure the port number was provided in the command line arguments.
   if ( argc < 2 ) {
      std::cout << "Usage: ./server <portno>\n";
      return 0;
   }
   
   // Store the port number provided by the user.
   portNo = ( int )strtol( argv[1], &endptr, 10 );
   if ( portNo <= 0 ) {
      perror ( "ERROR converting port number to integer " );
      return 1;
   }
   
   // Create socket for server. Could handle failure here. For now, just terminating.
   if ( ! createSocket( &serverDesc ) ) {
      perror ( "ERROR creating server socket " );
      return 1;
   }
   
   // Zero out the server/client socket address structure.
   bzero ( ( char * ) & serverAddr, sizeof( serverAddr ) );
   bzero ( ( char * ) & clientAddr, sizeof( clientAddr ) )
   ;
   // Set the socket family, port number, and IP address.
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons( portNo );
   serverAddr.sin_addr.s_addr = INADDR_ANY;
   
   // Bind the listener socket to an address.
   if ( ! bindSocket ( &serverDesc, &serverAddr ) ) {
      perror ( "ERROR binding socket to an address " );
      return 1;
   }
   
   // Start client listen-accept phase.
   while ( true ) {
      
      // Listen
      if ( ! listenSocket( &serverDesc ) ) {
         perror( "ERROR listening for socket connections " );
         return 1;
      }
      
      //Accept
      if ( ! acceptSocket( &serverDesc, &clientDesc, &clientAddr ) ) {
         perror( "ERROR accepting new socket connection " );
         return 1;
      }
      
      // Zero out the buffer to get ready to read from the socket.
      bzero( buffer, MAX_BUF );
      
      // Read bytes from the client socket.
      bytes = read ( clientDesc, buffer, MAX_BUF - 1 );
      if ( bytes < 0 ) {
         perror( "ERROR failed reading from socket descriptor " );
      }
      
      printf( "Received message: %s\n", buffer );
      
      // Write message to the client.
      char * msg = "Received your message\n";
      bytes = write (clientDesc, msg, sizeof( msg ) - 1 );
   }
   
   // Close client and server socket descriptors.
   close ( clientDesc );
   close ( serverDesc );

   return 0;
}  // End main()
