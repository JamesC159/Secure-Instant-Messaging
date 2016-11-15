#include <networking.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>
#include <string>

/*
 * Global Constants
 */
const int MAX_BUF = 8192;

/*
 * Structures
 */
struct clientData
{
   int tid;          // Thread ID
   int clientDesc;   // Client socket descriptor
};

/*
 * Helper functions
 */
void checkReadWrite( int bytes )
{
   // Reminder - stack buffer overflows. We need to do something about writing too
   // much or reading too much
   if ( bytes < 0 || bytes > MAX_BUF )
   {
      perror( "ERROR failed reading/writing to socket " );
   }
}

/*
 * Thread functions
 */
void * clientWorker ( void * in )
{
   
   int bytes = 0;                // Bytes read/written
   char buffer [ MAX_BUF ];      // Buffer to store message received from the client
   char msg [ MAX_BUF ];         // Message to send to the client
   struct clientData * cData;    // client thread data structure
   
   cData = (struct clientData *)in;
   bzero( buffer, MAX_BUF );
   
   printf( "Client thread %d starting\n", cData -> tid );
   
   bytes = read( cData -> clientDesc,
                 buffer,
                 MAX_BUF - 1 );
   
   checkReadWrite( bytes );
   
   printf( "Received message: %s\n", buffer );
   sprintf( msg, "Received your message %s", buffer );

   bytes = write( cData -> clientDesc,
                  msg,
                  sizeof( msg ) );
   
   checkReadWrite( bytes );
   
   printf( "Client thread %d exitting\n", cData -> tid );
}

/*
 * Main function
 *
 * TODO - validate the port number given on the command line.
 */
int
main ( int argc, char ** argv )
{
   
   pthread_t clientThread;          // Thread to spawn client workers
   socklen_t clientLen = 0;         // Length of the client sockaddr_in structure
   struct sockaddr_in serverAddr;   // Server socket address structure
   struct sockaddr_in clientAddr;   // Client socket address structure
   struct clientData * cData;       // Client worker thread data
   int serverDesc = -1;             // Server socket descriptor
   int clientDesc = -1;             // Client socket descriptor
   int portNo = -1;                 // Port number server is binded to
   int rc = 0;                      // pthread_create() return value
   int tcount = 1;                  // Thread id counter for each client
   
   // Make sure the port number was provided in the command line arguments.
   if ( argc < 2 )
   {
      fprintf( stderr, "Usage: ./server <portno>\n" );
      return 0;
   }
   
   // Validate and store the port number provided by the user.
   portNo = validatePort( argv[1] );
   if ( portNo == -1 )
   {
      if ( errno )
      {
         perror( "ERROR failed to convert port number argument to integer" );
      }
      else
      {
         fprintf( stderr, "ERROR invalid port number\n" );
      }
      
      return 1;
   }
   
   // Create socket for server. Could handle failure here. For now, just terminating.
   if ( ! createSocket( serverDesc ) )
   {
      perror( "ERROR creating server socket " );
      return 1;
   }
   
   // Zero out the server/client socket address structure.
   bzero( ( char * ) & serverAddr, sizeof( serverAddr ) );
   bzero( ( char * ) & clientAddr, sizeof( clientAddr ) );
   
   // Set the socket family, port number, and IP address.
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons( portNo );
   serverAddr.sin_addr.s_addr = INADDR_ANY;
   
   // Bind the listener socket to an address.
   if ( ! bindSocket( serverDesc, serverAddr ) )
   {
      perror ( "ERROR binding socket to an address " );
      return 1;
   }
   
   // Mark the server socket descriptor as a passive listener
   if ( ! listenSocket( serverDesc ) )
   {
      perror( "ERROR listening for socket connections " );
      return 1;
   }
   
   clientLen = sizeof( clientAddr );
   
   // Start client listen-accept phase.
   while ( true )
   {
      
      // Accept a client connection
      if ( ! acceptSocket(serverDesc,
                          clientDesc,
                          clientAddr,
                          clientLen ) )
      {
         perror( "ERROR failed to accept socket connection " );
         return 1;
      }
      
      // Create a new clientData structure.
      cData = new struct clientData;
      cData -> clientDesc = clientDesc;
      cData -> tid = tcount;
      
      // Spawn a worker thread for the connecting client.
      rc = pthread_create( &clientThread, NULL,
                          clientWorker, (void *) cData );
      if ( rc )
      {
         fprintf( stderr, "ERROR creating client thread : %d\n", rc );
         return 1;
      }
      
      tcount++;
   }
   
   close( clientDesc );
   close( serverDesc );

   return 0;
   
}  // End main()
