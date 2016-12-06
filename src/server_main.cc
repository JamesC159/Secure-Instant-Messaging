#include <Buddy.h>
#include <BuddyList.h>
#include <networking.h>
#include <serverhelp.h>
#include <ClientDB.h>
#include <ClientDB.h>
#include <csignal>

#include <fstream>
using std::ifstream;

struct ThreadData * tdata;
Socket sockListen;
Socket sockSource;
BuddyList buddylist;
ClientDB clientdb;

void InitClientDB( ifstream& );
void InitBuddyBuddy(ifstream& );
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
	  RSA::PublicKey publicKey;

	  // Load the private and public keys
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

	  // Read in the buddies from the buddies file.
	  ifstream buddyfile("buddies.txt");
	  if ( !buddyfile.is_open() )
	  {
		 throw(new Exception(Exception::IO_ERROR, "Failed to read buddies.txt"));
	  }

	  cout << "Reading content of buddy file" << endl;
	  InitBuddies(buddyfile);

	  // Read in the buddies from the buddies file.
	  	  ifstream buddybuddy("buddybuddy.txt");
	  	  if ( !buddybuddy.is_open() )
	  	  {
	  		 throw(new Exception(Exception::IO_ERROR, "Failed to read buddies.txt"));
	  	  }

	  	  cout << "Reading content of buddy file" << endl;
	  	  InitBuddyBuddy(buddybuddy);

	  // Read client authentication database
	  ifstream dbfile("db.txt");
	  if ( !dbfile.is_open() )
	  {
		 throw(new Exception(Exception::IO_ERROR, "Failed to read db.txt"));
	  }

	  cout << "Reading content of client database" << endl;
	  clientdb.InitClients(dbfile);

	  // Start client listen-accept phase.
	  while ( true )
	  {
		 Socket * cliSock;
		 cliSock = new Socket;
		 cliSock->Create();

		 sockListen.Accept(*cliSock, (sockaddr*) &clientaddr, &clientlen);
		 tdata = new struct ThreadData;
		 tdata->tid = tcount;
		 tdata->clientaddr = clientaddr;
		 tdata->clientlen = clientlen;
		 tdata->sockListen = sockListen;
		 tdata->sockSource = *cliSock;
		 tdata->privateKey = privateKey;

		 rc = pthread_create(&clientThread, NULL, clientWorker, (void *) tdata);
		 if ( rc )
		 {
			throw(new Exception(Exception::OTHER_ERROR,
			         "Failed creating client thread"));
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
   Buddy* client;

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
		 return (void*) -1;
	  }
	  else
	  {
		 authenticated = true;
		 cout << "Client successfully authenticated" << endl;
	  }

   }

   // Generate AES and CMAC keys
   AutoSeededRandomPool prng;

   SecByteBlock cmacKey(AES::DEFAULT_KEYLENGTH);
   prng.GenerateBlock(cmacKey, cmacKey.size());

   SecByteBlock aesKey(AES::DEFAULT_KEYLENGTH);
   prng.GenerateBlock(aesKey, aesKey.size());

   byte iv[ AES::BLOCKSIZE ];
   prng.GenerateBlock(iv, sizeof(iv));

   string plain;
   string mac, encoded;

   //Convert the keys into strings to send across the socket
   encoded.clear();
   StringSource(aesKey.data(), aesKey.size(), true, new StringSink(encoded)); // StringSource

   // Send AES Key
   SendMsg(encoded, tdata);

   encoded.clear();
   encoded = "";
   StringSource(iv, sizeof(iv), true, new StringSink(encoded));  // StringSource

   // Send IV
   SendMsg(encoded, tdata);

   encoded.clear();
   encoded = "";
   StringSource(cmacKey.data(), cmacKey.size(), true, new StringSink(encoded)); // StringSource

   // Send CMAC Key
   SendMsg(encoded, tdata);

   // Define the Encryptors and Decryptors for AES
   CBC_Mode< AES >::Encryption e;
   CBC_Mode< AES >::Decryption d;
   CMAC< AES > cmac(cmacKey, cmacKey.size());
   e.SetKeyWithIV(aesKey, aesKey.size(), iv);
   d.SetKeyWithIV(aesKey, aesKey.size(), iv);

   client = buddylist.FindBuddy(tdata->clientName);
   client->SetEnc(e);
   client->SetDec(d);
   client->SetCMAC(cmac);

   char readBuff[ 1500 ];
   char writeBuff[ 1500 ];
   memset(readBuff, 0, sizeof(readBuff));
   memset(writeBuff, 0, sizeof(writeBuff));
   string send = "";

   // Read BuddyList request and make sure it really was a buddylist request
   size_t bytes = symRead(d, cmac, &(tdata->sockSource), readBuff,
	        sizeof(readBuff));

   cout << "Bytes read: " << bytes << endl;

   int cliPort = (int) std::stol(readBuff);
   if ( cliPort > 0 )
   {
          cout << "Got port from " << client->username << ": " << cliPort << endl;
          sockaddr_in cAddr;
          socklen_t c_len = sizeof(cAddr);
          tdata->sockSource.GetPeerName((sockaddr *) &cAddr, &c_len);
          client->port = Integer(cliPort);
          client->ip = inet_ntoa(cAddr.sin_addr);
          cout << "Ip is: " << client->ip << endl;
   }
   else
   {
	  send = "FIN";
	  cout << "Invalid buddylist request..." << endl;
	  bytes = symWrite(e, cmac, &(tdata->sockSource), send.c_str(),
		       send.length());
	  return (void*) -1;
   }

   // Give the client their buddylist
   auto it = buddy2buddy.find(tdata->clientName);
   stringstream ss("");

   for (string buddy : it->second)
   {
	  ss << buddy << " ";
   }

   send = ss.str();

   bytes = symWrite(e, cmac, &(tdata->sockSource), send.c_str(), send.length());

   // Read request to talk to another client.
   memset(readBuff, 0, sizeof(readBuff));
   bytes = bytes = symRead(d, cmac, &(tdata->sockSource), readBuff,
	        sizeof(readBuff));

   // Search for the client in their BuddyList
   it = buddy2buddy.find(tdata->clientName);
   for (string buddy : it->second)
   {
	  if (buddy.compare(readBuff) == 0)
	  {
// Generate keys
// Generate ticket for 'B'
// Send keys and ticket to 'A'
// Ticket for 'B'
//	base64(enc(unameA base64(aesKey) base64(iv) base64(cmacKey)))
// Message to 'A'
// 	base64(enc(uanmeB ipB portB base64(aesKey) base64(iv) base64(cmacKey) ticket))
			SecByteBlock cmacKey(AES::DEFAULT_KEYLENGTH);
		    prng.GenerateBlock(cmacKey, cmacKey.size());

		    SecByteBlock aesKey(AES::DEFAULT_KEYLENGTH);
		    prng.GenerateBlock(aesKey, aesKey.size());

		    byte iv[ AES::BLOCKSIZE ];
		    prng.GenerateBlock(iv, sizeof(iv));

		    string plain;
		    string mac, encoded;

		    //Convert the keys into strings to send across the socket
		    encoded.clear();
		    StringSource(aesKey.data(), aesKey.size(), true, new StringSink(encoded)); // StringSource

		    // Send AES Key
		    SendMsg(encoded, tdata);

		    encoded.clear();
		    encoded = "";
		    StringSource(iv, sizeof(iv), true, new StringSink(encoded));  // StringSource

		    // Send IV
		    SendMsg(encoded, tdata);

		    encoded.clear();
		    encoded = "";
		    StringSource(cmacKey.data(), cmacKey.size(), true, new StringSink(encoded)); // StringSource

		    // Send CMAC Key
		    SendMsg(encoded, tdata);

		    // Define the Encryptors and Decryptors for AES
		    CBC_Mode< AES >::Encryption e;
		    CBC_Mode< AES >::Decryption d;
		    CMAC< AES > cmac(cmacKey, cmacKey.size());
		    e.SetKeyWithIV(aesKey, aesKey.size(), iv);
		    d.SetKeyWithIV(aesKey, aesKey.size(), iv);
	  }
   }

//   if ( buddy == nullptr )
//   {
//	  send = "FIN";
//	  cout << "Could not find buddy in buddylist" << endl;
//	  bytes = symWrite(e, cmac, &(tdata->sockSource), send.c_str(),
//		       send.length());
//	  return (void*) -1;
//   }

   bytes = symWrite(e, cmac, &(tdata->sockSource), send.c_str(), send.length());
   sockSource.ShutDown(SHUT_RDWR);


   return (void*) 0;
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

	  // Get username and password from client
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

	  tdata->clientName = uname;

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

	  //Forget nonce and salt
	  nonce = 0;
	  salt = 0;

	  return true;
   }
   catch ( CryptoPP::Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  return false;
   }
}

/******************************************************************************
 * FUNCTION:      InitBuddies
 * DESCRIPTION:   Read in the buddies.txt file.
 * PARAMETERS:    ifstream& file - buddies.txt file
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
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

/******************************************************************************
 * FUNCTION:      InitBuddies
 * DESCRIPTION:   Read in the buddies.txt file.
 * PARAMETERS:    ifstream& file - buddies.txt file
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void InitBuddyBuddy( ifstream& file )
{
   string line, user, buddy;
   vector<string> buddies;

   while ( getline(file, line) )
   {
	  stringstream ss(line);
	  ss >> user;
	  cout << "Username: " << user << endl;
	  while(ss >> buddy)
	  {
		 buddies.push_back(buddy);
	  }
	  ss.str("");
	  ss.clear();
	  buddy2buddy.insert(make_pair(user, buddies));
   }
}

