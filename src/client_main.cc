#include <networking.h>
#include <errno.h>
#include <iostream>
using std::cout;
using std::cin;
using std::endl;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;
using std::ostringstream;
using std::istringstream;

/******************************************************************************
 *                         MAIN FUNCTION
 *****************************************************************************/
int main(int argc, char ** argv ) 
{
   
   int portno = -1;
   
   if (argc < 2 )
   {
      fprintf( stderr, "Usage: client <portno>\n");
      return -1;
   }
   
   portno = validatePort( argv[1] );
   if ( portno == -1 )
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
   // Get listening socket and address
   /*
   int serverSock = socket (AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in sAddr;

   bzero ( (void *) &sAddr, sizeof(sAddr) );
   sAddr.sin_family = ( AF_INET );
   sAddr.sin_addr.s_addr = htonl ( INADDR_ANY );
   sAddr.sin_port = htons ( 0 );

   if(bind( serverSock, (struct sockaddr *) & sAddr, sizeof(sAddr)))
   {
       fprintf(stderr, "Node %d failed to bind listening socket.\n", c->nodeID);
       exit ( -1 );
   }
   else
   {
       fprintf(stdout, 
               "Node %d bound listening socket successfully.\n", 
               c->nodeID);
   }

   listen ( serverSock, 20);
   */
   // connect to server
   try
   {
   	AutoSeededRandomPool rng;
   	RSA::PublicKey serverKey;
   	LoadPublicKey("rsa-public.key", serverKey);
   	serverKey.Validate(rng, 3);
   	
		Socket sockServer;
		sockServer.Create();
		sockServer.Connect("localhost", portno);

		// login - agree on session key with server
		// get buddy list from server
		// wait on user for request to comm with another client
		// mutual authentication between requested client
		// derive encryption/authentication keys from session key
		// let user talk (and display messages from other user)
		// disconnect

		while( true )
		{
			string sendBuf, recBuf, temp, recovered;
			ostringstream ss;
			Integer m, c, r;
			byte byteBuf[500];
			
			cout << "Enter secret to send to server: ";
			if(!getline(cin, sendBuf))
			{
				cout << "Failed" << endl;
			}
				
			cout << endl;
			
			// Encode the message as an Integer
			m = Integer((const byte *)sendBuf.c_str(), sendBuf.size());
			
			//Encrypt
			c = serverKey.ApplyFunction(m);
			
			//Turn the encrypted value into a string
			ss << c;
			sendBuf = ss.str();
			ss.str(string());
			cout << "m: " << sendBuf << endl;
		   sockServer.Send((const byte *)sendBuf.c_str(), sendBuf.size());
		   
		   
		   // Retrieve message from socket
			sockServer.Receive(byteBuf, sizeof(byteBuf));
			cout << endl << "m received from the server: " << byteBuf << endl;
		
			// Convert message to a string
			ss << byteBuf;
			recBuf = ss.str();
			ss.str(string());
			
			//Convert the string to an Integer so we can calculate the inverse
			c = Integer(recBuf.c_str());
    		r = serverKey.ApplyFunction(c);
    		cout << "r: " << r << endl;
    	
    		// Recover the original message
    		size_t req = r.MinEncodedSize();
			recovered.resize(req);
			r.Encode((byte *)recovered.data(), recovered.size());
			cout << "recovered: " << recovered << endl;

		}
		
		sockServer.CloseSocket();
	}
	catch(Exception& e)
	{
		cout << "Exception caught: " << e.what() << endl;
	}
}
