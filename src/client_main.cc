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

	  RSASS<PSSR, SHA256>::Verifier verifier(serverKey);

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

	  // Convert username, digestt, and nonce to string and send
	  ss << uname << "~" << encoded << "~" << nonce;
	  cout << "Final String to be Sent: " << ss.str() << endl;
	  sendBuf = ss.str();
	  ss.str("");
	  ss.clear();
	  sendMsg(sockServer, sendBuf);

	  ifstream dhfile("dh.txt");
	  if ( !dhfile.is_open() )
	  {
		 throw("Failed to authenticate");
	  }

	  string line;
	  getline(dhfile, line);
	  Integer p = Integer(line.c_str());
	  getline(dhfile, line);
	  Integer g = Integer(line.c_str());
	  getline(dhfile, line);
	  Integer q = Integer(line.c_str());

	  DH dh;
	  AutoSeededRandomPool rnd;
	  dh.AccessGroupParameters().Initialize(p, q, g);
	  if ( !dh.GetGroupParameters().ValidateGroup(rnd, 3) )
	  {
		 throw runtime_error("Failed to validate prime and generator");
	  }

	  size_t count = 0;

	  p = dh.GetGroupParameters().GetModulus();
	  q = dh.GetGroupParameters().GetSubgroupOrder();
	  g = dh.GetGroupParameters().GetGenerator();

	  Integer v = ModularExponentiation(g, q, p);
	  if ( v != Integer::One() )
	  {
		 throw runtime_error("Failed to verify order of the subgroup");
	  }

	  DH2 dhB(dh);

	  SecByteBlock sprivB(dhB.StaticPrivateKeyLength()), spubB(
		       dhB.StaticPublicKeyLength());
	  SecByteBlock eprivB(dhB.EphemeralPrivateKeyLength()), epubB(
		       dhB.EphemeralPublicKeyLength());

	  dhB.GenerateStaticKeyPair(rnd, sprivB, spubB);
	  dhB.GenerateEphemeralKeyPair(rnd, eprivB, epubB);

	  string sbEncoded, ebEncoded;
	  StringSource sbSource(spubB, sizeof(spubB), true,
		       new HexEncoder(new StringSink(sbEncoded)));
	  StringSource ebSource(epubB, sizeof(epubB), true,
		       new HexEncoder(new StringSink(ebEncoded)));


	  // Get spubA and epubA from server
//	  byte byteBuf[ 10000 ];
//	  cout << "Waiting for reply from server..." << endl;
//	  	  sockServer.Receive(byteBuf, sizeof(byteBuf));
//	  	  cout << endl << "Cipher Received: " << byteBuf << endl;
//
//	  	  // Convert message to a string
//	  	  ss << byteBuf;
//	  	  recBuf = ss.str();
//	  	  ss.str("");
//	  	  ss.clear();
	  string sa = recoverMsg(sockServer);
	  string ea = recoverMsg(sockServer);

	  //Calculate shared secret.

	  cout << "SA: " << sa << endl << "EA: " << ea << endl;
	  cout << "Size of SA: " << sa.size() << endl;
	  cout << "Size of EA: " << ea.size() << endl;

	  string encoding;
	  string decodedSA, decodedEA;

	  StringSource decodeSA(sa, true,
		        new HexDecoder( new StringSink(decodedSA)));

	  StringSource decodeEA(ea, true,
	  	  		       new HexDecoder(new StringSink(decodedEA)));

	  SecByteBlock spubA((const byte*) decodedSA.data(), decodedSA.size());
	  SecByteBlock epubA((const byte*) decodedEA.data(), decodedEA.size());

	  if ( spubA.size() < dhB.StaticPublicKeyLength() ) spubA.CleanGrow(
		       dhB.StaticPublicKeyLength());
	  else spubA.resize(dhB.StaticPublicKeyLength());

	  if ( epubA.size() < dhB.EphemeralPublicKeyLength() ) epubA.CleanGrow(
		       dhB.EphemeralPublicKeyLength());
	  else epubA.resize(dhB.EphemeralPublicKeyLength());

	  cout << "epubA Size: " << epubA.size() << endl;
	  cout << "spubA Size: " << spubA.size() << endl;

	  cout << "Agreed Value Length: " << dhB.AgreedValueLength() << endl;

	  SecByteBlock sharedB(dhB.AgreedValueLength());

	  if ( !dhB.Agree(sharedB, sprivB, eprivB, spubA, epubA) )
	  {
		 throw runtime_error("Failed to reach shared secret (B)");
	  }

	  cout << "Sending SB Encoded: " << sbEncoded << endl;
	  cout << "Sending EB Encoded: " << ebEncoded << endl;
	  sendMsg(sockServer, sbEncoded);
	  sendMsg(sockServer, ebEncoded);

	  Integer a, b;
	  b.Decode(sharedB.BytePtr(), sharedB.SizeInBytes());
	  cout << "Shared secret (B): " << b << endl;

//
//	  count = std::min(dhA.AgreedValueLength(), dhB.AgreedValueLength());
//	  if ( !count || 0 != memcmp(sharedA.BytePtr(), sharedB.BytePtr(), count) ) throw runtime_error(
//		       "Failed to reach shared secret");

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
