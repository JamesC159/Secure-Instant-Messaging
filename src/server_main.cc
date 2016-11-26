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
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::flush;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;
using std::istringstream;
using std::ostringstream;

/*
 * Structures.
 */
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
void readFromClient( int &, string );
void writeToClient( int &, string );
bool authenticate ( struct clientThreadData ** );

/*
 * Thread functions.
 */
void * clientWorker ( void * );

/*
 * Globals.
 */
Socket sockListen;
Socket sockSource;

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
   
     try
   {

		sockListen.Create();
		sockSource.Create();
   	sockListen.Bind(portNo);
   	sockListen.Listen();
	
	
		///////////////////////////////////////
		// Pseudo Random Number Generator
		AutoSeededRandomPool rng;

		// Below, the values for d and e were swapped
		Integer n("0xbeaadb3d839f3b5f"), e("0x11"), d("0x21a5ae37b9959db9");

		RSA::PrivateKey privateKey;
		privateKey.Initialize(n, d, e);

		RSA::PublicKey publicKey;
		publicKey.Initialize(n, d);
		
      SavePublicKey("rsa-public.key", publicKey);
      SavePrivateKey("rsa-private.key", privateKey);

		 
	    // Start client listen-accept phase.
	    while ( true )
	    {

	       sockListen.Accept(sockSource);
	       
	       // Create a new clientThreadData structure.
	       cData = new struct clientThreadData;
	       cData -> sockDesc = clientDesc;
	       cData -> tid = tcount;
			 cData -> privateKey = privateKey;
			 cData -> publicKey = publicKey;
			 //cData -> sockListen = sockListen;
			 //cData -> sockSource = sockSource;
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
	    
	    sockListen.CloseSocket();
   }
   catch( CryptoPP::Exception& e ) 
	{
       std::cerr << "Error: " << e.what() << std::endl;
   }

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
                  string buffer )
{
   int bytes = 0;

   bytes = read( socket,
                  (char*)buffer.c_str(),
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
                  string msg )
{
   int bytes = 0;

   bytes = write( socket,
                  (char*)msg.c_str(),
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

   // First we must authenticate the client. This involves establishing the
   // session key between the client and the server.
   while (true) {
   //if( ! authenticated )
   //{
      bool res = authenticate( &cData );   
   //}
   }

	//sockSource.CloseSocket();
   
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
	try
	{
		string recovered, recBuf, temp, sendBuf;
		ostringstream ss;
		Integer c, r, m;
		AutoSeededRandomPool rng;
		byte byteBuf [MAX_BUF];
		
		// Retrieve message from socket
		sockSource.Receive(byteBuf, sizeof(byteBuf));
		cout << byteBuf << endl;
		
		// Convert message to a string
		ss << byteBuf;
		recBuf = ss.str();
		ss.str(string());
		
		//Convert the string to an Integer so we can calculate the inverse
		c = Integer(recBuf.c_str());
    	r = (*cData)->privateKey.CalculateInverse(rng, c);
    	cout << "c: " << c << endl << "r: " << r << endl;
    	
    	// Recover the original message
    	size_t req = r.MinEncodedSize();
		recovered.resize(req);
		r.Encode((byte *)recovered.data(), recovered.size());
		cout << "recovered: " << recovered << endl;
		
//////////////////////// SENDING ///////////////////////////////////////////////
		
		// Get secret to send
		cout << "Reply back to the client: ";
		
		if(!getline(cin, sendBuf))
		{
			cout << "Failed" << endl;
		}
		cout << endl;
			
		
		// Encode the message as an Integer
		m = Integer((const byte *)sendBuf.c_str(), sendBuf.size());
		
		//Encrypt
		c = (*cData)->privateKey.CalculateInverse(rng, m);
		
		//Turn the encrypted value into a string 
		ss << c;
		sendBuf = ss.str();
		ss.str(string());
		cout << "c: " << sendBuf << endl;
	   sockSource.Send((const byte *)sendBuf.c_str(), sendBuf.size());
			   	
	   // Read username, hash(password, salt), nonce from client.
	   // Check information against clientDB.
	   // Generate Diffie-Hellman number.
	   // Read client Diffie-Hellman number.eeeeeeeeeee
	   // Calculate Ksa = Diffie-Hellman key.
	}
   catch( CryptoPP::Exception& e )
   {
       cerr << "caught Exception..." << endl;
       cerr << e.what() << endl;
   }
}
