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
	  byte byteBuf[ 10000 ];
	  memset(byteBuf, 0, sizeof(byteBuf));

	  // Retrieve message from socket
	  cout << "Waiting to receive a message from client " << tdata->tid << endl;

//	  RSAES_OAEP_SHA_Decryptor d(tdata->privateKey);

	  size_t bytes = tdata->sockSource.Receive(byteBuf, sizeof(byteBuf));
	  cout << "Bytes Read: " << bytes << endl;

	  cout << "Encoded Cipher Received: " << byteBuf << endl;

	  string decodedCipher;
	  StringSource(byteBuf, sizeof(byteBuf), true, new Base64Decoder(new StringSink(decodedCipher)));

	  c = Integer(decodedCipher.c_str());

	  // Decrypt
	  r = tdata->privateKey.CalculateInverse(rng, c);
	  cout << "r: " << r << endl;

	  // Round trip the message
	  size_t req = r.MinEncodedSize();
	  recovered.resize(req);
	  r.Encode((byte *) recovered.data(), recovered.size());

	  cout << "Recovered: " << recovered << endl;

//	  cout << "recovered: " << recovered << endl;
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
	  	  m = Integer((const byte *) sendBuf.c_str(), sendBuf.size());
	  	  cout << "m: " << m << endl;

	  	  // Encrypt
	  	  c = tdata->privateKey.CalculateInverse(rng, m);

	  	  ss << c;
	  	  string cipher = ss.str();
	  	  ss.str("");
	  	  ss.clear();

	  	  string encodedCipher;
	  	  StringSource(cipher, cipher.size(), new Base64Encoder(new StringSink(encodedCipher)));

	  	cout << "Encoded Cipher Sent: " << encodedCipher << endl;

	  size_t bytes = tdata->sockSource.Send((const byte*)encodedCipher.c_str(), encodedCipher.size());
	  cout << "Bytes Written: " << bytes << endl;

	  sleep(2);
   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  tdata->sockSource.ShutDown(SHUT_RDWR);
   }
}

int symWrite( CBC_Mode< AES >::Encryption eAES, CMAC< AES > cmac, Socket * sock,
         const char * buff, int len )
{
   string cipher = "";
   string encoded = "";
   string mac;
   size_t bytes, originalBytes;
   StringSource s((const byte*)buff, len, true,
	        new StreamTransformationFilter(eAES, new StringSink(cipher)) // StreamTransformationFilter
	                 );// StringSource

   StringSource(cipher, true, new Base64Encoder(new StringSink(encoded)) // Base64Encoder
	        );// StringSource

   // Send Cipher with CMAC to Server (Request for BuddyList)
   cout << "Symmetric Cipher: " << encoded << endl;

   originalBytes = sock->Send((const byte*) encoded.c_str(), encoded.size());

   byte temp[ 10 ];
   sock->Receive(temp, sizeof(temp));

   StringSource((const byte*)buff, len, true, new HashFilter(cmac, new StringSink(mac)) // HashFilter
	        );// StringSource

   encoded = "";
   StringSource(mac, true, new Base64Encoder(new StringSink(encoded)) // HexEncoder
	        );// StringSource

   cout << "Cipher Text Buddy List MAC: " << encoded << endl;

   bytes = sock->Send((const byte*) encoded.c_str(), encoded.size());

   return originalBytes;

}

int symRead( CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket * sock,
         char * buff, int len )
{
   string decoded, recovered;
   char * tempBuf[ 10000 ];
   memset(tempBuf, 0, sizeof(tempBuf));
   const int flags = HashVerificationFilter::THROW_EXCEPTION
	        | HashVerificationFilter::HASH_AT_END;

   size_t bytes = sock->Receive((byte*) buff, len);

   StringSource((const byte *)buff, bytes, true, new Base64Decoder(new StringSink(decoded)) // Base64Encoder
	        );// StringSource

   // The StreamTransformationFilter removes
   //  padding as required.
   StringSource s(decoded, true,
	        new StreamTransformationFilter(dAES, new StringSink(recovered)) // StreamTransformationFilter
	                 );// StringSource

   cout << "Symmetric Recovered: " << recovered << endl;

   string ack = "ACK";

   bytes = sock->Send((const byte*) ack.c_str(), ack.size());

   bytes = sock->Receive((byte*) tempBuf, sizeof(tempBuf));

   decoded = "";
   StringSource((const byte*)tempBuf, bytes, true, new Base64Decoder(new StringSink(decoded)) // Base64Encoder
	        );// StringSource

   string recoveredPlusTemp = recovered + decoded;

   StringSource(recoveredPlusTemp, true, new HashVerificationFilter(cmac, NULL, flags)); // StringSource

   cout << "Verified message" << endl;

   memset(buff, 0, len);
   memcpy(buff, recovered.c_str(), recovered.length());

   return recovered.length();
}
