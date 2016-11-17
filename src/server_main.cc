/*
 * Assumptions:   (1) The buddy list is already known in advance
 *                (2) The username and passwords of clients are already known
 *                    in advance.
 * TODO:
 */

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
 * Structures.
 */
struct clientThreadData
{
   int tid;          // Thread ID.
   int sockDesc;   // Client socket descriptor.
};

struct clientDB
{
   char * username;
   char * pwHash;
   int salt;
};
/*
 * Helper functions.
 */
void checkReadWrite( int );
void processRequest( char * );
void readFromClient( int &, char * );
void writeToClient( int &, char * );
bool authenticate ( struct clientThreadData ** );
/*
 * Thread functions.
 */
void * clientWorker ( void * );
/*
 * Global Constants.
 */
const char * FIN_STR = "FIN\n\0";   // These flags can be whatever we want
                                    // them to be.
const char * SYN_STR = "SYN\n\0";
const char * RST_STR = "RST\n\0";
const int MAX_BUF = 8192;
/*
 * Global Variables.
 */
struct clientDB db;
int nonce;
// Private RSA key
// Public RSA key
/******************************************************************************
 *                            MAIN FUNCTION       
 *****************************************************************************/
int
main ( int argc, char ** argv )
{

   pthread_t clientThread;          // Thread to spawn client workers.
   socklen_t clientLen = 0;         // Length of the client sockaddr_in
                                    //    structure.
   struct sockaddr_in serverAddr;   // Server socket address structure.
   struct sockaddr_in clientAddr;   // Client socket address structure.
   struct clientThreadData * cData; // Client worker thread data.
   struct buddy * buddylist;        // List of buddies stored on the server.
   int serverDesc = -1;             // Server socket descriptor.
   int clientDesc = -1;             // Client socket descriptor.
   int portNo = -1;                 // Port number server is binded to.
   int rc = 0;                      // pthread_create() return value.
   int tcount = 1;                  // Thread id counter for each client.
   
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
   
   // Create socket for server.
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
   
   // Mark the server socket descriptor as a passive listener.
   if ( ! listenSocket( serverDesc ) )
   {
      perror( "ERROR listening for socket connections " );
      return 1;
   }
   
   clientLen = sizeof( clientAddr );
   
   // Start client listen-accept phase.
   while ( true )
   {

      // Accept a client connection.
      if ( ! acceptSocket( serverDesc,
                           clientDesc,
                           clientAddr,
                           clientLen ) )
      {
         perror( "ERROR failed to accept socket connection " );
         return 1;
      }
      
      // Create a new clientThreadData structure.
      cData = new struct clientThreadData;
      cData -> sockDesc = clientDesc;
      cData -> tid = tcount;

      // Spawn a worker thread for the connecting client.
      rc = pthread_create( &clientThread, 
                           NULL,
                           clientWorker, 
                           (void *) cData );
      if ( rc )
      {
         fprintf( stderr, "ERROR creating client thread : %d\n", rc );
         return 1;
      }
      
      tcount++;
   }
   
   close( serverDesc );

   return 0;
   
}
/******************************************************************************
 * FUNCTION:      checkReadWrite  
 * DESCRIPTION:   Validates socket reads and writes
 * PARAMETERS:    int bytes
 *                   - number of bytes read or written to a socket descriptor.
 * RETURN:        None  
 * NOTES:      
 *****************************************************************************/
void 
checkReadWrite( int bytes )
{
   // Reminder - stack buffer overflows. We need to do something about writing
   // too much or reading too much.
   if ( bytes < 0 || bytes > MAX_BUF )
   {
      perror( "ERROR failed reading/writing to socket " );
   }
}
/******************************************************************************
 * FUNCTION:      readFromClient  
 * DESCRIPTION:   reads bytes written to socket descriptor from client
 * PARAMETERS:    char ** buffer
 *                   - message buffer
 *                struct clientThreadData ** cData
 *                   - structure containing client socket/thread info
 * RETURN:        None
 * NOTES:      
 *****************************************************************************/
void
readFromClient( int & socket, 
                  char * buffer )
{
   int bytes = 0;

   bytes = read( socket,
                  buffer,
                  MAX_BUF );
   checkReadWrite( bytes );

}
/******************************************************************************
 * FUNCTION:      readFromClient  
 * DESCRIPTION:   reads bytes written to socket descriptor from client
 * PARAMETERS:    char ** buffer
 *                   - message buffer
 *                struct clientThreadData ** cData
 *                   - structure containing client socket/thread info
 * RETURN:        None
 * NOTES:      
 *****************************************************************************/
void
writeToClient( int & socket, 
                  char * msg )
{
   int bytes = 0;

   bytes = write( socket,
                  msg,
                  MAX_BUF );
   checkReadWrite( bytes );

}
/******************************************************************************
 * FUNCTION:      clientWorker    
 * DESCRIPTION:   Thread worker function for accepted client socket 
 *                connections.
 * PARAMETERS:    void * in - clientThreadData structure
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void * 
clientWorker ( void * in )
{

   int bytes = 0;                // Bytes read/written.
   bool FIN = false;             // Flag from the client for closing the
                                 //    connection.
   bool authenticated  = false;  // Flag for client authentication recognition.
   char buffer [ MAX_BUF ];      // Buffer to store message received from the
                                 //    client.
   char msg [ MAX_BUF ];         // Message to send to the client.
   char * action;                // Action to take after processing the client
                                 //    client request.
   struct clientThreadData * cData;    // Client thread data structure.
   
   cData = (struct clientThreadData *)in;
   printf( "Client thread %d starting\n", cData -> tid );

   memset( buffer, '\0', sizeof( buffer ) );

      // Read from client.
   readFromClient( cData -> sockDesc, buffer );

      // First we must authenticate the client. This involves establishing the
      // session key between the client and the server.
   if( ! authenticated )
   {
      authenticate( &cData );
      authenticated = true;
   }
   
   while( ! FIN )
   {
      memset( buffer, '\0', sizeof( buffer ) );

      // Read from client.
      readFromClient( cData -> sockDesc, buffer );

      // Process Request.
      processRequest( buffer );

      if ( strcmp( buffer, FIN_STR ) == 0 )
      {
         FIN = true;
      }

      printf( "Received message: %s\n", buffer );  // Note there are 2 '\n'
                                                   // characters here since
                                                   // the buffer is being
                                                   // filled from client stdin.
      sprintf( msg, "Received your message %s", buffer );

      // Write ACK to client.
      writeToClient( cData -> sockDesc,
                     msg );
   }

   printf( "Client thread %d exitting\n", cData -> tid );
   close( cData -> sockDesc );
}
/******************************************************************************
 * FUNCTION:      processRequest  
 * DESCRIPTION:   Determine client requests to server.
 * PARAMETERS:    char * request 
 *                   - The client request to the server
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void 
processRequest( char * request )
{
   // Check if client session request.
   //    Agree on session key for client-client comm.
   //    Get ticket to the requested client and send it.
   // Check if buddy list request.
   // Check if client wants to close the connection.

}
/******************************************************************************
 * FUNCTION:      authenticate   
 * DESCRIPTION:   Authenticates and establishes Diffie-Hellman session key
 *                between a client and the server.   
 * PARAMETERS:    struct clientThreadData ** cData 
 *                   - socket descriptor and thread id for the client.
 * RETURN:        true if client successfully authenticated.
 *                false if client authentication was a failure.
 * NOTES:       
 *****************************************************************************/
bool
authenticate( struct clientThreadData ** cData )
{
   int bytes = 0;
   char buffer [ MAX_BUF ];
   // Read client username.
   readFromClient( (*cData) -> sockDesc,
                     buffer );
   // Generate random nonce and salt.
   // Write random nonce and salt to client.
   // Read username, hash(password, salt), nonce from client.
   // Check information against clientDB.
   // Generate Diffie-Hellman number.
   // Read client Diffie-Hellman number.
   // Calculate Ksa = Diffie-Hellman key.
}
