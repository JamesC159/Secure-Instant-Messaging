#include <Buddy.h>
#include <BuddyList.h>
#include <networking.h>
#include <serverhelp.h>
#include <ClientDB.h>
#include <ClientDB.h>
#include <csignal>

#include <fstream>
using std::ifstream;

const int BUDDIES = 15;
const string FIN_STR = "FIN";
const string SYN_STR = "SYN";
const string RST_STR = "RST";
struct ThreadData * tdata;
Socket sockListen;
Socket sockSource;
BuddyList buddylist;
ClientDB clientdb;

void processRequest( char * );
void InitClientDB( ifstream& );
void InitBuddies( ifstream& );
bool authenticate( struct ThreadData * );
void * clientWorker( void * );
void SignalHandler( int signum )
{
   cout << "Abort signal (" << signum << ") received.\n";

   try
   {
	  sockListen.ShutDown(SHUT_RDWR);
	  sockSource.ShutDown(SHUT_RDWR);
   }
   catch ( Exception& e )
   {
	  cerr << e.what() << endl;
   }

   exit(signum);

}

/******************************************************************************
 *                            MAIN FUNCTION       
 *****************************************************************************/
int main( int argc, char ** argv )
{

   pthread_t clientThread;
   int rc = 0;
   int tcount = 1;
   sockaddr_in clientaddr;
   socklen_t clientlen = sizeof(clientaddr);
   signal(SIGTERM, SignalHandler);

   try
   {
	  sockListen.Create();
	  sockSource.Create();
	  sockListen.Bind(port);
	  sockListen.Listen();

	  AutoSeededRandomPool rng;
	  RSA::PrivateKey privateKey;
	  RSA::PublicKey publicKey;

	  LoadPrivateKey("rsa-private.key", privateKey);
	  LoadPublicKey("rsa-public.key", publicKey);

	  // Read in the buddies from the buddies file.
	  ifstream buddyfile("buddies.txt");
	  if ( !buddyfile.is_open() )
	  {
		 cerr << "Failed to read in buddies" << endl;
		 return -1;
	  }

	  ifstream dbfile("db.txt");
	  if ( !dbfile.is_open() )
	  {
		 cerr << "Failed to read in buddies" << endl;
		 return -1;
	  }

	  cout << "Reading content of buddy file" << endl;
	  InitBuddies(buddyfile);

	  cout << "Reading content of client database" << endl;
	  clientdb.InitClients(dbfile);

	  // Start client listen-accept phase.
	  while ( true )
	  {

		 sockListen.Accept(sockSource, (sockaddr*) &clientaddr, &clientlen);
		 tdata = new struct ThreadData;
		 tdata->tid = tcount;
		 tdata->clientaddr = clientaddr;
		 tdata->clientlen = clientlen;
		 tdata->sockListen = sockListen;
		 tdata->sockSource = sockSource;
		 tdata->publicKey = publicKey;
		 tdata->privateKey = privateKey;

		 rc = pthread_create(&clientThread, NULL, clientWorker, (void *) tdata);
		 if ( rc )
		 {
			fprintf(stderr, "ERROR creating client thread : %d\n", rc);
			return 1;
		 }
		 tcount++;
	  }

	  sockListen.ShutDown(SHUT_RDWR);
   }
   catch ( CryptoPP::Exception& e )
   {
	  cerr << "ERROR: " << e.what() << endl;
   }
   catch ( const char* e )
   {
	  cerr << "ERROR: " << e << endl;
   }

   return 0;

}

/******************************************************************************
 * FUNCTION:      clientWorker    
 * DESCRIPTION:   Thread worker function for accepted client socket 
 *                connections.
 * PARAMETERS:    void * in - ThreadData structure
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void * clientWorker( void * in )
{

   bool fin = false;
   bool authenticated = false; // Flag for client authentication recognition.
   string buffer;
   struct ThreadData * tdata; // Client thread data structure.

   tdata = (struct ThreadData *) in;
   cout << "Client thread " << tdata->tid << " Starting..." << endl;

   // First we must authenticate the client. This involves establishing the
   // session key between the client and the server.
   if ( !authenticated )
   {
	  fin = authenticate(tdata);
	  if ( !fin )
	  {
		 cout << "Client did not authenticate" << endl;
		 sockSource.ShutDown(SHUT_RDWR);
		 throw("Client did not authenticate");
	  }
	  else
	  {
		 authenticated = true;
		 cout << "Client successfully authenticated" << endl;
	  }

   }

   cout << "Beginning session with client " << tdata->tid << "..." << endl;
   fin = false;
   while ( !fin )
   {
	  buffer = RecoverMsg(tdata);
	  cout << "Enter ACK to client " << tdata->tid << ": ";
	  if ( !getline(cin, buffer) )
	  {
		 cout << "Failed" << endl;
	  }
	  SendMsg(buffer, tdata);
   }

   sockSource.ShutDown(SHUT_RDWR);
   return (void*) 0;

}

/******************************************************************************
 * FUNCTION:      processRequest  
 * DESCRIPTION:   Determine client requests to server.
 * PARAMETERS:    char * request 
 *                   - The client request to the server
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void processRequest( char * request )
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
 * PARAMETERS:    struct ThreadData ** tdata
 *                   - socket descriptor and thread id for the client.
 * RETURN:        true if client successfully authenticated.
 *                false if client authentication was a failure.
 * NOTES:       
 *****************************************************************************/
bool authenticate( struct ThreadData * tdata )
{
   try
   {
	  string sendBuf, recvBuf, uname, password;

	  stringstream ss("");
	  AutoSeededRandomPool rng;
	  Integer nonce, salt;
	  SHA256 hash;
	  string digestStr;
	  string nstr;
	  string tnonce;

	  // Get username and passwordfrom client
	  recvBuf = RecoverMsg(tdata);
	  ss.str(recvBuf);
	  ss >> uname >> password;
	  salt = clientdb.FindUser(uname);
	  ss.str("");
	  ss.clear();
	  if ( salt == -1 )
	  {
		 cerr << "Client was not found in the database" << endl;
		 sendBuf = "FIN";
		 SendMsg(sendBuf, tdata);
		 return false;
	  }

	  /*Buddy* buddy = buddylist.FindBuddy(uname);
	   if ( buddy == nullptr )
	   {
	   sendBuf = "FIN";
	   SendMsg(sendBuf, tdata);
	   return false;
	   }*/

	  // Send Nonce and Salt
	  nonce = Integer(rng, 64);
	  ss << nonce << " " << salt;
	  sendBuf = ss.str();
	  ss.str("");
	  ss.clear();
	  cout << "Sending Nonce and Salt to Client: " << nonce << " " << salt
		       << endl;

	  SendMsg(sendBuf, tdata);

	  // Receive username, hash(pw, salt) and nonce
	  recvBuf = RecoverMsg(tdata);
	  ss.str(recvBuf);

	  // Parse the reply
	  getline(ss, uname, '~');
	  getline(ss, digestStr, '~');
	  getline(ss, tnonce, '~');
	  ss.str("");
	  ss.clear();

	  cout << "Digest string" << digestStr;

//	  if ( nonce != Integer(tnonce.c_str()) )
//	  {
//		 sendBuf = "FIN";
//		 SendMsg(sendBuf, tdata);
//		 return false;
//	  }

	  if ( !clientdb.MatchDigest(uname, digestStr) )
	  {
		 cerr << "Failed to match the digest: " << digestStr << endl;
		 sendBuf = "FIN";
		 SendMsg(sendBuf, tdata);
		 return false;
	  }

	  // Check username/password against client authentication database

	  return true;
   }
   catch ( CryptoPP::Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  return false;
   }
   catch ( const char* e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e << endl;
	  return false;
   }
}

void InitBuddies( ifstream& file )
{
   string line, user, port;
   while ( getline(file, line) )
   {
	  stringstream ss(line);
	  string user, port;

	  ss >> user >> port;
	  cout << "Username: " << user << " Port: " << port << endl;
	  Integer portno = Integer(port.c_str());
	  buddylist.AddBuddy(user, portno);
   }
}

