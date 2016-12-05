/*
 * clienthelp.cc
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#include "clienthelp.h"
#include <sys/socket.h>

RSA::PublicKey serverKey;

string recoverMsg( Socket& sockServer )
{
   try
   {
	  string recovered = "";
	  AutoSeededRandomPool rng;
	  byte byteBuf[ 10000 ];
	  string recBuf;
	  stringstream ss;
	  Integer c, r, m;
	  memset(byteBuf, 0, sizeof(byteBuf));

	  // Retrieve message from socket
	  cout << "Waiting for reply from server..." << endl;

	  size_t bytes = sockServer.Receive(byteBuf, sizeof(byteBuf));
	  cout << "Bytes Read: " << bytes << endl;

	  cout << "Encoded Cipher Received: " << byteBuf << endl;

	  string decodedCipher;
	  	  StringSource(byteBuf, sizeof(byteBuf), true, new Base64Decoder(new StringSink(decodedCipher)));

	  	  c = Integer(decodedCipher.c_str());

	  // Decrypt
	  r = serverKey.ApplyFunction(c);
	  cout << "r: " << r << endl;

	  // Round trip the message
	  size_t req = r.MinEncodedSize();
	  recovered.resize(req);
	  r.Encode((byte *) recovered.data(), recovered.size());
	  cout << "Recovered: " << recovered << endl;
	  return recovered;
   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  sockServer.ShutDown(SHUT_RDWR);
	  return "";
   }
}

void sendMsg( Socket& sockServer, string sendBuf )
{
   try
   {
	  AutoSeededRandomPool rng;
	  stringstream ss;
	  Integer m, c, r;

	  // Treat the message as a big endian array
	  m = Integer((const byte *) sendBuf.data(), sendBuf.size());
	  cout << "m: " << m << endl;

	  // Encrypt
	  c = serverKey.ApplyFunction(m);
	  ss << c;
	  string cipher = ss.str();
	  ss.str("");
	  ss.clear();

	  string encodedCipher;
	  	  	  StringSource(cipher, cipher.size(), new Base64Encoder(new StringSink(encodedCipher)));

	  	  	cout << "Encoded Cipher Sent: " << encodedCipher << endl;


	  size_t bytes = sockServer.Send((const byte*) encodedCipher.c_str(), encodedCipher.size());
	  cout << "Bytes Written: " << bytes << endl;
	  sleep(2);

   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  sockServer.ShutDown(SHUT_RDWR);
   }
}
