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
createSocket ( int & sockDesc ) {
   
   sockDesc = socket ( AF_INET,
                       SOCK_STREAM,
                       0 );
   
   if ( sockDesc < 0 ) {
      return false;
   }
   
   return true;
}

bool
bindSocket ( int & sockDesc,
                 struct sockaddr_in & sockAddr ) {
   
   if ( bind( sockDesc,
             (struct sockaddr *) & sockAddr,
             (socklen_t)sizeof( sockAddr )) < 0 ) {
      
      return false;
   }
   
   return true;
}

bool
listenSocket ( int & sockDesc ) {
   
   if ( listen ( sockDesc, MAX_CONN ) < 0 ) {
      return false;
   }
   
   return true;
}

bool
acceptSocket ( int & servDesc,
               int & cliDesc,
               struct sockaddr_in & cliAddr,
               socklen_t & cliLen ) {
   
   cliDesc = accept( servDesc,
                    (struct sockaddr *) & cliAddr,
                     & cliLen );
   
   if ( cliDesc < 0 ) {
      return false;
   }
   
   return true;
}

// main()
int
main ( int argc, char ** argv ) {
   
   struct sockaddr_in serverAddr;   // Server socket address structure
   struct sockaddr_in clientAddr;   // Client socket address structure
   int serverDesc = -1;             // Server socket descriptor
   int clientDesc = 0;              // Client socket descriptor
   int bytes = 0;                   // Bytes read or written to sockets
   int portNo = -1;                 // Port number server is binded to
   socklen_t clientLen = 0;         // Length of the client sockaddr_in structure
   char buffer [ MAX_BUF ];         // Buffer for reading from the client socket
   char * endptr;                   // Ptr for error handling in strtol()
   char * msg;                      // Message to write to the client socket
   
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
   if ( ! createSocket( serverDesc ) ) {
      perror ( "ERROR creating server socket " );
      return 1;
   }
   
   // Zero out the server/client socket address structure.
   bzero ( ( char * ) & serverAddr, sizeof( serverAddr ) );
   bzero ( ( char * ) & clientAddr, sizeof( clientAddr ) );
   
   // Set the socket family, port number, and IP address.
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons( portNo );
   serverAddr.sin_addr.s_addr = INADDR_ANY;
   
   // Bind the listener socket to an address.
   if ( ! bindSocket ( serverDesc, serverAddr ) ) {
      perror ( "ERROR binding socket to an address " );
      return 1;
   }
   
   // Listen
   if ( ! listenSocket( serverDesc ) ) {
      perror( "ERROR listening for socket connections " );
      return 1;
   }
   
   // Store length of the client
   clientLen = sizeof( clientAddr );
   
   // Start client listen-accept phase.
   while ( true ) {
      
      if ( ! acceptSocket(serverDesc,
                          clientDesc,
                          clientAddr,
                          clientLen ) ) {
         perror( "ERROR failed to accept socket connection " );
         return 1;
      }
      
      //Accept
      if ( clientDesc < 0 ) {
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
      msg = (char *)calloc( MAX_BUF, sizeof( msg ) );
      msg = "Received your message";
      bytes = write (clientDesc, msg, sizeof( msg ) - 1 );
      if ( bytes < 0 ) {
         perror( "ERROR: failed writing to socket " );
         return 1;
      }
      free ( msg );
   }
   
   // Close client and server socket descriptors.
   close ( clientDesc );
   close ( serverDesc );

   return 0;
   
}  // End main()
