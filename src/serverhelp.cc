/*
 * serverhelp.cc
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#include "serverhelp.h"

/******************************************************************************
 * FUNCTION:      RecoverMsg
 * DESCRIPTION:   Recovers a Base64 encoded message from a client, decodes it,
 * 				  and then decrypts it to recover the original message.
 * PARAMETERS:    struct ThreadData* tdata - ThreadData structure
 * RETURN:        string recovered - The decrypted and recovered message.
 * NOTES:
 *****************************************************************************/
string RecoverMsg( struct ThreadData * tdata )
{
   try
   {
	  Integer c = 0, r = 0, m = 0;
	  size_t req = 0, bytes = 0;
	  AutoSeededRandomPool rng;
	  string recovered = "", ack = "", decodedCipher = "";
	  byte * byteBuf;
	  memset(byteBuf, 0, sizeof(byteBuf));

	  // Retrieve message from socket
	  cout << "Waiting to receive a message from client " << tdata->tid << endl;
          int recieveLen;
          tdata->sockSource.Receive((byte*) &recieveLen, sizeof(int));
          byteBuf = new byte [recieveLen+1];
          memset(byteBuf, 0, recieveLen+1);

	  bytes = tdata->sockSource.Receive(byteBuf, sizeof(byteBuf));
	  cout << "Bytes Read: " << bytes << endl;

	  cout << "Encoded Cipher Received: " << byteBuf << endl;

	  decodedCipher = "";
	  StringSource(byteBuf, sizeof(byteBuf), true,
		       new Base64Decoder(new StringSink(decodedCipher)));

	  c = Integer(decodedCipher.c_str());

	  // Decrypt
	  r = tdata->privateKey.CalculateInverse(rng, c);
	  cout << "r: " << r << endl;

	  // Round trip the message
	  req = r.MinEncodedSize();
	  recovered.resize(req);
	  r.Encode((byte *) recovered.data(), recovered.size());

	  cout << "Recovered: " << recovered << endl;

	  return recovered;
   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  tdata->sockSource.ShutDown(SHUT_RDWR);
   }
}

/******************************************************************************
 * FUNCTION:      SendMsg
 * DESCRIPTION:   Encrypt and Base64 encodes a string to send over a client
 * 					socket connection.
 * PARAMETERS:    string sendBuf- string to encode and encrypt
 * 				  struct ThreadData* tdata - ThreadData structure
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void SendMsg( string sendBuf, struct ThreadData * tdata )
{
   try
   {
	  AutoSeededRandomPool rng;
	  stringstream ss("");
	  string cipher = "", encodedCipher = "";
	  Integer m = 0, c = 0, r = 0;
	  size_t bytes = 0;

	  // Treat the message as a big endian array
	  m = Integer((const byte *) sendBuf.c_str(), sendBuf.size());
	  cout << "m: " << m << endl;

	  // Encrypt
	  c = tdata->privateKey.CalculateInverse(rng, m);

	  ss << c;
	  cipher = ss.str();
	  ss.str("");
	  ss.clear();

	  // Base64 encode the cipher
	  encodedCipher = "";
	  StringSource(cipher, cipher.size(),
		       new Base64Encoder(new StringSink(encodedCipher)));

	  cout << "Encoded Cipher Sent: " << encodedCipher << endl;
          int sendLen = encodedCipher.size();

          tdata->sockSource.Send((const byte*) &sendLen, sizeof(int));
	  // Send the cipher
	  bytes = tdata->sockSource.Send((const byte*) encodedCipher.c_str(),
		       encodedCipher.size());
	  cout << "Bytes Written: " << bytes << endl;
   }
   catch ( Exception& e )
   {
	  cerr << "caught Exception..." << endl;
	  cerr << e.what() << endl;
	  tdata->sockSource.ShutDown(SHUT_RDWR);
   }
}

/******************************************************************************
 * FUNCTION:      symWrite
 * DESCRIPTION:   Base64 encodes and encrypts a message using AES to send over
 * 					socket connection.
 * PARAMETERS:    CBC_Mode< AES >::Encryption eAES - symmetric AES key
 * 				  CMAC< AES > cmac - Message Authentication key
 * 				  Socket* socket - Pointer to client socket connection
 * 				  const char* buff - Buffer containing data to write
 * 				  int len - Length of the buffer
 * RETURN:        int originalBytes - Number of bytes written when sending
 * 					the encrypted message (not the MAC of the message)
 * NOTES:
 *****************************************************************************/
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

   int sendLen = encoded.size();

   sock->Send((const byte*) sendLen, sizeof(int));

   originalBytes = sock->Send((const byte*) encoded.c_str(), encoded.size());

   StringSource((const byte*)buff, len, true, new HashFilter(cmac, new StringSink(mac)) // HashFilter
	        );// StringSource

   encoded = "";
   StringSource(mac, true, new Base64Encoder(new StringSink(encoded)) // HexEncoder
	        );// StringSource

   cout << "Cipher Text MAC: " << encoded << endl;

   sendLen = encoded.size();

   sock->Send((const byte*) sendLen, sizeof(int));

   bytes = sock->Send((const byte*) encoded.c_str(), encoded.size());

   return originalBytes;

}

/******************************************************************************
 * FUNCTION:      symRead
 * DESCRIPTION:   Base64 decodes and decrypts a message using AES key.
 * PARAMETERS:    CBC_Mode< AES >::Decryption dAES - AES decryption key
 * 				  CMAC< AES > cmac - CMAC authentication key
 * 				  Socket* sock - Pointer to client socket
 * 				  char* buff - Buffer that holds recovered message
 * 				  int len - Length of buff
 * RETURN:        int recovered.length() - Length of the recovered message.
 * NOTES:
 *****************************************************************************/
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
   sock->Receive((byte*) recieveLen, sizeof(int));
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
   sock->Receive((byte*) recieveLen, sizeof(int));
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
