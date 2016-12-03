#include <networking.h>
#include <clienthelp.h>
#include <csignal>
#include <errno.h>

#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::getline;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;
using std::stringstream;
using std::stringstream;

const string FIN_STR = "FIN"; // These flags can be whatever we want them to be.
const string SYN_STR = "SYN";
const string RST_STR = "RST";
Socket sockServer;

void ParseNonceSalt( Integer&, Integer&, stringstream& );

void SignalHandler( int signum )
{
   cout << "Abort signal (" << signum << ") received.\n";

   try
   {
	  sockServer.ShutDown(SHUT_RDWR);
   }
   catch ( Exception& e )
   {
	  cerr << e.what() << endl;
   }

   exit(signum);

}

/******************************************************************************
 *                         MAIN FUNCTION
 *****************************************************************************/
int main( int argc, char ** argv )
{
   signal(SIGTERM, SignalHandler);

   try
   {
	  AutoSeededRandomPool rng;
	  RSA::PublicKey serverKey;
	  LoadPublicKey("rsa-public.key", serverKey);
	  if ( !serverKey.Validate(rng, 3) )
	  {
		 cerr << "Failed to load public RSA key";
		 return -1;
	  }

	  sockServer.Create();
	  sockServer.Connect("localhost", port);

	  string sendBuf, recBuf, temp, uname, password, recovered;
	  Integer m, c, r;

	  // Enter username and password to login. Allow 3 tries.

	  cout << "Enter your username: ";
	  if ( !getline(cin, sendBuf) )
	  {
		 cout << "Failed" << endl;
	  }

	  uname = sendBuf;	// Store username

	  cout << "Enter your password: ";
	  if ( !getline(cin, sendBuf) )
	  {
		 cout << "Failed" << endl;
	  }

	  password = sendBuf;
	  sendBuf = uname + " " + password;

	  sendMsg(serverKey, sockServer, sendBuf);
	  recovered = recoverMsg(serverKey, sockServer);

	  // Make sure the server did not reply with connection teardown info
	  if ( recovered.compare(FIN_STR) == 0 )
	  {
		 cout << "Since receiving the FIN from server, we are exiting" << endl;
		 sockServer.ShutDown(SHUT_RDWR);
		 return -1;
	  }

	  //Parse nonce and salt reply
	  stringstream ss(recovered);
	  Integer nonce, salt;
	  ParseNonceSalt(nonce, salt, ss);
	  ss.str("");
	  	  ss.clear();

	  // Prepare to hash pw with salt and send username and nonce with it.
	  byte digest[ SHA256::DIGESTSIZE ];
	  SHA256 hash;

	  //Convert password into byte*
	  byte* password_p = (byte*) password.c_str();
	  unsigned int pwlen = password.length();

	  // Convert salt to byte*
	  ss << salt;
	  temp = ss.str();
	  byte* salt_p = (byte*) temp.c_str();
	  unsigned int saltlen = temp.length();

	  //Calculate SHA256 hash
	  hash.Update(password_p, pwlen);        //hash password first
	  hash.Update(salt_p, saltlen);        //then hash salt
	  hash.Final(digest);               //final result
	  ss.str("");
	  	  ss.clear();

	  //Convert the digest to hex.
	  string encoded;
	  StringSource strsrc(digest, sizeof(digest), true,
		       new HexEncoder(new StringSink(encoded)) // HexEncoder
		                );// StringSource

	  cout << "Encoded hash of digest: " << encoded << endl;
	  cout << "Sending Hash Stuff" << endl;

	  // Convert username, digestt, and nonce to string and send
	  ss << uname << "~" << encoded << "~" << nonce;
	  cout << "Final String to be Sent: " << ss.str() << endl;
	  sendBuf = ss.str();
	  ss.str("");
	  	  ss.clear();
	  sendMsg(serverKey, sockServer, sendBuf);
	  sleep(5);	// So server can receive just in case.

	  sockServer.ShutDown(SHUT_RDWR);
   }
   catch ( Exception& e )
   {
	  cout << "Exception caught: " << e.what() << endl;
	  sockServer.ShutDown(SHUT_RDWR);
   }
}

void ParseNonceSalt( Integer& nonce, Integer& salt, stringstream& ss )
{
   ss >> nonce >> salt;
   cout << "Nonce: " << nonce << endl << "Salt: " << salt << endl;
}
