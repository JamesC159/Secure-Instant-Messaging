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
	  byte byteBuf[ 1000 ];
	  string recBuf;
	  stringstream ss;
	  Integer c, r, m;

	  // Retrieve message from socket
	  cout << "Waiting for reply from server..." << endl;

	  size_t bytes = sockServer.Receive(byteBuf, sizeof(byteBuf));

	  ss << byteBuf;
	  recBuf = ss.str();

	  // Retrieve message from socket
//	  RSAES_OAEP_SHA_Decryptor d(tdata->privateKey);
//	  tdata->sockSource.Receive(byteBuf, sizeof(byteBuf));

//Convert message to a string

	  c = Integer(recBuf.c_str());

	  // Decrypt
	  r = serverKey.ApplyFunction(c);
	  cout << "r: " << r << endl;

	  // Round trip the message
	  size_t req = r.MinEncodedSize();
	  recovered.resize(req);
	  r.Encode((byte *) recovered.data(), recovered.size());

	  //cout << "recovered: " << recovered << endl;
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

	  sockServer.Send((const byte*) cipher.c_str(), cipher.size());

   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  sockServer.ShutDown(SHUT_RDWR);
   }
}
