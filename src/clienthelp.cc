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
	  string recovered = "", decodedCipher = "", ack = "";
	  AutoSeededRandomPool rng;
	  byte byteBuf[ 2000 ];
	  Integer c = 0, r = 0, m = 0;
	  size_t bytes = 0, req = 0;
	  memset(byteBuf, 0, sizeof(byteBuf));

	  // Retrieve message from socket
	  cout << "Waiting for reply from server..." << endl;

	  bytes = sockServer.Receive(byteBuf, sizeof(byteBuf));
	  cout << "Bytes Read: " << bytes << endl;

	  cout << "Encoded Cipher Received: " << byteBuf << endl;

	  StringSource(byteBuf, sizeof(byteBuf), true,
		       new Base64Decoder(new StringSink(decodedCipher)));

	  c = Integer(decodedCipher.c_str());

	  // Decrypt
	  r = serverKey.ApplyFunction(c);
	  cout << "r: " << r << endl;

	  // Round trip the message
	  req = r.MinEncodedSize();
	  recovered.resize(req);
	  r.Encode((byte *) recovered.data(), recovered.size());
	  cout << "Recovered: " << recovered << endl;

	  ack = "ACK";
	  bytes = sockServer.Send((const byte*) ack.c_str(),
	  		       ack.size());

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
	  stringstream ss("");
	  string cipher = "", encodedCipher = "";
	  Integer m = 0, c = 0, r = 0;
	  size_t bytes = 0;
	  byte ack[10];
	  memset(ack, 0, sizeof(ack));

	  // Treat the message as a big endian array
	  m = Integer((const byte *) sendBuf.c_str(), sendBuf.size());
	  cout << "m: " << m << endl;

	  // Encrypt
	  c = serverKey.ApplyFunction(m);
	  ss << c;
	  cipher = ss.str();
	  ss.str("");
	  ss.clear();

	  // Base64 encode so we can not worry about the buffers
	  StringSource(cipher, cipher.size(),
		       new Base64Encoder(new StringSink(encodedCipher)));

	  cout << "Encoded Cipher Sent: " << encodedCipher << endl;

	  // Send Encoded cipher
	  bytes = sockServer.Send((const byte*) encodedCipher.c_str(),
		       encodedCipher.size());

	  cout << "Bytes Written: " << bytes << endl;

	  // Get acknowledgement, just for sync purposes
	  bytes = sockServer.Receive(ack, sizeof(ack));

   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  sockServer.ShutDown(SHUT_RDWR);
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

   int sendLen = enconded.size();

   sock->Send((const byte*) sendLen, sizeof(int));

   originalBytes = sock->Send((const byte*) encoded.c_str(), encoded.size());

   StringSource((const byte*)buff, len, true, new HashFilter(cmac, new StringSink(mac)) // HashFilter
	        );// StringSource

   encoded = "";
   StringSource(mac, true, new Base64Encoder(new StringSink(encoded)) // HexEncoder
	        );// StringSource

   cout << "Cipher Text MAC: " << encoded << endl;

   sendLen = enconded.size();

   sock->Send((const byte*) sendLen, sizeof(int));

   bytes = sock->Send((const byte*) encoded.c_str(), encoded.size());

   return originalBytes;

}

int symRead( CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket * sock,
         char * buff, int len )
{
   string decoded = "", recovered = "", ack = "", recoveredPlusTemp = "";
   size_t bytes = 0;
   char * tempBuf;
   memset(tempBuf, 0, sizeof(tempBuf));
   const int flags = HashVerificationFilter::THROW_EXCEPTION
	        | HashVerificationFilter::HASH_AT_END;

   int recieveLen;
   sock->Recieve((byte*) recieveLen, sizeof(int));
   tempBuf = new char [recieveLen+1];
   memset(tempBuf, 0, recieveLen+1);
   bytes = sock->Receive((byte*) tempBuf, len);

   StringSource((const byte*) tempBuf, bytes, true, new Base64Decoder(new StringSink(decoded)) // Base64Encoder
	        );// StringSource

   delete [] tempBuf;
   // The StreamTransformationFilter removes
   //  padding as required.
   StringSource s(decoded, true,
	        new StreamTransformationFilter(dAES, new StringSink(recovered)) // StreamTransformationFilter
	                 );// StringSource

   cout << "Symmetric Recovered: " << recovered << endl;

   bytes = sock->Receive((byte*) tempBuf, sizeof(tempBuf));
   sock->Recieve((byte*) recieveLen, sizeof(int));
   tempBuf = new char [recieveLen+1];
   memset(tempBuf, 0, recieveLen+1);
   bytes = sock->Receive((byte*) tempBuf, len);


   decoded = "";
   StringSource((const byte*)tempBuf, bytes, true, new Base64Decoder(new StringSink(decoded)) // Base64Encoder
	        );// StringSource

   recoveredPlusTemp = recovered + decoded;

   StringSource(recoveredPlusTemp, true, new HashVerificationFilter(cmac, NULL, flags)); // StringSource

   cout << "Verified message" << endl;

   memset(buff, 0, len);
   memcpy(buff, recovered.c_str(), recovered.length());
   delete [] tempBuf;
   return recovered.length();
}
