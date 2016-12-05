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
   signal(SIGINT, SignalHandler);

   try
   {
	  sockListen.Create();
	  sockSource.Create();
	  sockListen.Bind(port);
	  sockListen.Listen();

	  AutoSeededRandomPool rng;
	  RSA::PrivateKey privateKey;
//	  privateKey.GenerateRandomWithKeySize(rng, 2048);
//	  privateKey.Initialize(Integer(0x9dd403188c219481d10cba1c3f6af8a55b0e103bc4abb1e0f81391df5dd9307e8b5c75904296bacca0041dfc0795983cd5e9a0a0da2521554f250b9239f6ab90f1b13a1d5031fae2fc002a9030c18faa141c568d961c3ae67cd61d5040d78b01c808e9251963f2dbe459f158ab38e8614ee9f676d1be3cd94799f7edf1042f83734edac69c0fe2914153b26fb090a75f1b114b2a48462b2c03874aa93cea7f45562c4a3aa9f29bf1cb6af2a0056a346203e46536efa227dfeb92a7039ab4f6eedcfc30c6246360dd5980a7059435c1970196d6907d7dc37ebaf1755eecc733c6323c0a7137e8d93ae28dd8a02e8cfb9af08b0773dc5b3936c31a4d7cb9ed544f),
//			   Integer(0x2e6b8870a1af8608104f098fd66ac19a0bb8d7991bba07240cba7632a321c2f80adef566aa2c550f01e317e0b6efc35d2fdb4d5c7c65460a083812764d488cd0471602089f1dc242c296a31b59a2576e422673cf4a4489cb51e49f35b8b7dd971cb753a17ff0385ecab10abfb9e38fa42644d004d44702d67e789436ec88c2adba390b9336e006efd6a0c32a6dc4b0a0dda2e053ca66ee92d5842ce57dc6f34375e29cda145c2fcec11becea6c898eb8c06d10575a524e5ca9672170eb37fe2b713780ddae93dcb778a3a3ff1de85267124fbd0d6689796a53551020cf26eecb11856ce74401b73cdfc016515458c8f6f89661313e32521562e3319150c00719),
//			   Integer(0x11));
	  RSA::PublicKey publicKey; //(privateKey);
//	  publicKey.Initialize(Integer(0x9dd403188c219481d10cba1c3f6af8a55b0e103bc4abb1e0f81391df5dd9307e8b5c75904296bacca0041dfc0795983cd5e9a0a0da2521554f250b9239f6ab90f1b13a1d5031fae2fc002a9030c18faa141c568d961c3ae67cd61d5040d78b01c808e9251963f2dbe459f158ab38e8614ee9f676d1be3cd94799f7edf1042f83734edac69c0fe2914153b26fb090a75f1b114b2a48462b2c03874aa93cea7f45562c4a3aa9f29bf1cb6af2a0056a346203e46536efa227dfeb92a7039ab4f6eedcfc30c6246360dd5980a7059435c1970196d6907d7dc37ebaf1755eecc733c6323c0a7137e8d93ae28dd8a02e8cfb9af08b0773dc5b3936c31a4d7cb9ed544f),
//			   Integer(0x2e6b8870a1af8608104f098fd66ac19a0bb8d7991bba07240cba7632a321c2f80adef566aa2c550f01e317e0b6efc35d2fdb4d5c7c65460a083812764d488cd0471602089f1dc242c296a31b59a2576e422673cf4a4489cb51e49f35b8b7dd971cb753a17ff0385ecab10abfb9e38fa42644d004d44702d67e789436ec88c2adba390b9336e006efd6a0c32a6dc4b0a0dda2e053ca66ee92d5842ce57dc6f34375e29cda145c2fcec11becea6c898eb8c06d10575a524e5ca9672170eb37fe2b713780ddae93dcb778a3a3ff1de85267124fbd0d6689796a53551020cf26eecb11856ce74401b73cdfc016515458c8f6f89661313e32521562e3319150c00719)
//);
//	  SavePrivateKey("rsa-private.key", privateKey);
//	  SavePublicKey("rsa-public.key", publicKey);

	  LoadPrivateKey("rsa-private.key", privateKey);
	  if ( !privateKey.Validate(rng, 3) )
	  {
		 cerr << "Failed to load server private RSA key";
		 return -1;
	  }

	  LoadPublicKey("rsa-public.key", publicKey);
	  if ( !publicKey.Validate(rng, 3) )
	  {
		 cerr << "Failed to load server public RSA key";
		 return -1;
	  }

	  // Signer object
	  RSASS< PSSR, SHA256 >::Signer signer(privateKey);

	  // Create signature space
	  size_t length = signer.MaxSignatureLength();
	  SecByteBlock signature(length);

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
		 tdata->signer = signer;
		 tdata->privateKey = privateKey;

		 rc = pthread_create(&clientThread, NULL, clientWorker, (void *) tdata);
		 if ( rc )
		 {
			fprintf(stderr, "ERROR creating client thread : %d\n", rc);
			return 1;
		 }
		 tcount++;
	  }
   }
   catch ( CryptoPP::Exception& e )
   {
	  sockListen.ShutDown(SHUT_RDWR);
	  sockSource.ShutDown(SHUT_RDWR);
	  cerr << "ERROR: " << e.what() << endl;
   }
   catch ( const char* e )
   {
	  sockListen.ShutDown(SHUT_RDWR);
	  sockSource.ShutDown(SHUT_RDWR);
	  cerr << "ERROR: " << e << endl;
   }
   catch ( const std::exception& e )
   {
	  sockListen.ShutDown(SHUT_RDWR);
	  sockSource.ShutDown(SHUT_RDWR);
	  cerr << e.what() << endl;
	  return -1;
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
		 throw("Client did not authenticate");
	  }
	  else
	  {
		 authenticated = true;
		 cout << "Client successfully authenticated" << endl;
	  }

   }

   AutoSeededRandomPool prng;

   SecByteBlock cmacKey(AES::DEFAULT_KEYLENGTH);
   prng.GenerateBlock(cmacKey, cmacKey.size());

   SecByteBlock aesKey(AES::DEFAULT_KEYLENGTH);
   prng.GenerateBlock(aesKey, aesKey.size());

   byte iv[ AES::BLOCKSIZE ];
   prng.GenerateBlock(iv, sizeof(iv));

   string plain;
   string mac, encoded;

   /*********************************\
   	\*********************************/

   encoded.clear();
   StringSource(aesKey.data(), aesKey.size(), true,
	       new StringSink(encoded) // HexEncoder
	                 );// StringSource

   // Send AES Key
   SendMsg(encoded, tdata);

   encoded.clear();
   encoded = "";
   StringSource(iv, sizeof(iv), true, new StringSink(encoded) // HexEncoder
	        );// StringSource

   // Send IV
   SendMsg(encoded, tdata);

   encoded.clear();
   encoded = "";
   StringSource(cmacKey.data(), cmacKey.size(), true,
	       new StringSink(encoded) // HexEncoder
	                 );// StringSource

   // Send CMAC Key
   SendMsg(encoded, tdata);

   try
   {
	  CBC_Mode< AES >::Encryption e;
	  CBC_Mode< AES >::Decryption d;
	  CMAC< AES > cmac(cmacKey, cmacKey.size());

	  e.SetKeyWithIV(aesKey, aesKey.size(), iv);
	  d.SetKeyWithIV(aesKey, aesKey.size(), iv);

	  char readBuff[1024];
	  char writeBuff[1024];
	  memset(readBuff, 0, sizeof(readBuff));
	  memset(writeBuff, 0, sizeof(writeBuff));

	  size_t bytes = symRead(d, cmac, &(tdata->sockSource), readBuff, sizeof(readBuff) );
	  cout << "Bytes read: " << bytes << endl;

	  if (strcmp(readBuff, "GetBuddyList") == 0)
	  {
		 cout << "Received the buddy list!" << endl;
	  }
   }
   catch ( Exception& e )
   {
	  throw(e);
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
//	   StringSource ss1(rec, sizeof(rec), true, new StringSink(recvBuf));

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
//	   StringSource ss2(rec, sizeof(rec), true, new StringSink(recvBuf));
	  ss.str(recvBuf);

	  // Parse the reply
	  getline(ss, uname, '~');
	  getline(ss, digestStr, '~');
	  getline(ss, tnonce, '~');
	  ss.str("");
	  ss.clear();

	  if ( nonce != Integer(tnonce.c_str()) )
	  {
		 sendBuf = "FIN";
		 SendMsg(sendBuf, tdata);
		 return false;
	  }

	  if ( !clientdb.MatchDigest(uname, digestStr) )
	  {
		 cerr << "Failed to match the digest: " << digestStr << endl;
		 sendBuf = "FIN";
		 SendMsg(sendBuf, tdata);
		 return false;
	  }

	  return true;
   }
   catch ( CryptoPP::Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
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

