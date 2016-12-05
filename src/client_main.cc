#include <networking.h>
#include <clienthelp.h>
#include <csignal>
#include <errno.h>

#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::getline;

#include <fstream>
using std::ifstream;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;
using std::stringstream;
using std::stringstream;

#include <stdexcept>
using std::runtime_error;

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
   signal(SIGINT, SignalHandler);

   try
   {
	  AutoSeededRandomPool rng;
	  LoadPublicKey("rsa-public.key", serverKey);
	  if ( !serverKey.Validate(rng, 3) )
	  {
		 cerr << "Failed to load public RSA key";
		 return -1;
	  }

	  RSASS< PSSR, SHA256 >::Verifier verifier(serverKey);

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

	  sendMsg(sockServer, sendBuf);
	  recovered = recoverMsg(sockServer);

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

	  // Convert username, digest, and nonce to string and send
	  ss << uname << "~" << encoded << "~" << nonce;
//	  cout << "Final String to be Sent: " << ss.str() << endl;
	  sendBuf = ss.str();
	  ss.str("");
	  ss.clear();
	  sendMsg(sockServer, sendBuf);

	  //Get shared key from server
	  string aes_str = recoverMsg(sockServer);
	  string iv_str = recoverMsg(sockServer);
	  string cmac_str = recoverMsg(sockServer);

	  cout << "AES Key: " << aes_str << endl;
	  cout << "IV: " << iv_str << endl;
	  cout << "CMAC Key: " << cmac_str << endl;

	  SecByteBlock cmac_key(AES::DEFAULT_KEYLENGTH);
	  SecByteBlock server_key(AES::DEFAULT_KEYLENGTH);
	  byte iv[ AES::BLOCKSIZE ];

	  ArraySource((const byte*) aes_str.c_str(), aes_str.size(), true,
		       new HexDecoder(new ArraySink(server_key, sizeof(server_key))));

	  ArraySource((const byte*) iv_str.c_str(), iv_str.size(), true,
		       new HexDecoder(new ArraySink(iv, sizeof(iv))));

	  ArraySource((const byte*) cmac_str.c_str(), cmac_str.size(), true,
		       new HexDecoder(new ArraySink(cmac_key, sizeof(cmac_key))));

	  try
	  {

		 CBC_Mode< AES >::Encryption e;
		 CBC_Mode< AES >::Decryption d;
		 CMAC< AES > cmac(cmac_key, cmac_key.size());
		 const int flags = HashVerificationFilter::THROW_EXCEPTION
			      | HashVerificationFilter::HASH_AT_END;

		 e.SetKeyWithIV(server_key, server_key.size(), iv);
		 d.SetKeyWithIV(server_key, server_key.size(), iv);

		 // The StreamTransformationFilter removes
		 //  padding as required.
		 string plain = "GetBuddyList";
		 string cipher;
		 string mac;
		 string encoded;

		 StringSource s(plain, true,
			      new StreamTransformationFilter(e, new StringSink(cipher)) // StreamTransformationFilter
			               );// StringSource

		 // Pretty print
		 encoded.clear();
		 StringSource(cipher, true, new HexEncoder(new StringSink(encoded)) // HexEncoder
			      );// StringSource

		 // Send Cipher with CMAC to Server (Request for BuddyList)
		 cout << "Cipher Text Buddy List Request: " << encoded << endl;

		 size_t bytes = sockServer.Send((const byte*) encoded.c_str(), encoded.size());

		 sleep(2);

		 StringSource(plain, true, new HashFilter(cmac, new StringSink(mac)) // HashFilter
			      );// StringSource

		 // Pretty print
		 encoded.clear();
		 StringSource(mac, true, new HexEncoder(new StringSink(encoded)) // HexEncoder
			      );// StringSource

		 cout << "Cipher Text Buddy List MAC: " << encoded << endl;

		 bytes = sockServer.Send((const byte*) encoded.c_str(), encoded.size());
	  }
	  catch ( const CryptoPP::Exception& e )
	  {
		 cerr << e.what() << endl;
		 sockServer.ShutDown(SHUT_RDWR);
		 exit(1);
	  }

	  sockServer.ShutDown(SHUT_RDWR);
   }
   catch ( Exception& e )
   {
	  // Send message to server here so they can shut down the socket
	  // correctly.
	  cout << "Exception caught: " << e.what() << endl;
	  sockServer.ShutDown(SHUT_RDWR);
	  return -1;
   }
   catch ( const std::exception& e )
   {
	  cerr << e.what() << endl;
	  sockServer.ShutDown(SHUT_RDWR);
	  return -1;
   }
}

void ParseNonceSalt( Integer& nonce, Integer& salt, stringstream& ss )
{
   ss >> nonce >> salt;
   cout << "Nonce: " << nonce << endl << "Salt: " << salt << endl;
}
