/*
 * serverhelp.cc
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#include "serverhelp.h"

string RecoverMsg( struct ThreadData * tdata )
{
   try
   {
	  string recBuf;
	  stringstream ss;
	  Integer c, r, m;
	  AutoSeededRandomPool rng;
	  string recovered;
	  byte byteBuf[ 1000 ];

	  // Retrieve message from socket
	  cout << "Waiting to receive a message from client " << tdata->tid << endl;

//	  RSAES_OAEP_SHA_Decryptor d(tdata->privateKey);

	  tdata->sockSource.Receive(byteBuf, sizeof(byteBuf));

	  //Convert message to a string
	  ss << byteBuf;
	  recBuf = ss.str();
	  ss.str("");
	  ss.clear();

	  c = Integer(recBuf.c_str());

	  // Decrypt
	  r = tdata->privateKey.CalculateInverse(rng, c);
	  cout << "r: " << r << endl;

	  // Round trip the message
	  size_t req = r.MinEncodedSize();
	  recovered.resize(req);
	  r.Encode((byte *) recovered.data(), recovered.size());

	  cout << "recovered: " << recovered << endl;
	  return recovered;

   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  tdata->sockSource.ShutDown(SHUT_RDWR);
   }
}

void SendMsg( string sendBuf, struct ThreadData * tdata )
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
	  	  c = tdata->privateKey.CalculateInverse(rng, m);
	  	  ss << c;
	  	  string cipher = ss.str();
	  	  ss.str("");
	  	  ss.clear();

	  size_t bytes = tdata->sockSource.Send((const byte*)cipher.c_str(), cipher.size());
	  cout << "Bytes Written: " << bytes << endl;
   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  tdata->sockSource.ShutDown(SHUT_RDWR);
   }
}
