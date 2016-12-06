/*
 * clienthelp.cc
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#include "clienthelp.h"
#include <sys/socket.h>

BuddyList buddylist;

RSA::PublicKey serverKey;

string recoverMsg( Socket& sockServer )
{
   try
   {
	  string recovered = "", decodedCipher = "", ack = "";
	  AutoSeededRandomPool rng;
	  byte * byteBuf;
	  Integer c = 0, r = 0, m = 0;
	  size_t bytes = 0, req = 0;
          int receiveLen;
          sockServer.Receive((byte*) &receiveLen, sizeof(int));
          byteBuf = new byte [receiveLen+1];
          memset(byteBuf, 0, receiveLen+1);

	  // Retrieve message from socket
	  cout << "Waiting for reply from server..." << endl;

	  bytes = sockServer.Receive(byteBuf, receiveLen);
	  cout << "Bytes Read: " << bytes << endl;

	  cout << "Encoded Cipher Received: " << byteBuf << endl;

	  StringSource(byteBuf, receiveLen, true,
		       new Base64Decoder(new StringSink(decodedCipher)));

	   delete [] byteBuf;

	  c = Integer(decodedCipher.c_str());

	  // Decrypt
	  r = serverKey.ApplyFunction(c);
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

	  int sendSize = encodedCipher.size();
	  sockServer.Send((const byte *) &sendSize, sizeof(int));

	  cout << "Send size: " << sendSize << endl;

	  // Send Encoded cipher
	  bytes = sockServer.Send((const byte*) encodedCipher.c_str(),
		       encodedCipher.size());

	  cout << "Bytes Written: " << bytes << endl;
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

   int sendLen = encoded.size();

   sock->Send((const byte*) &sendLen, sizeof(int));
   originalBytes = sock->Send((const byte*) encoded.c_str(), sendLen);

   StringSource((const byte*)buff, len, true, new HashFilter(cmac, new StringSink(mac)) // HashFilter
	        );// StringSource

   encoded = "";
   StringSource(mac, true, new Base64Encoder(new StringSink(encoded)) // HexEncoder
	        );// StringSource

   cout << "Cipher Text MAC: " << encoded << endl;

   sendLen = encoded.size();
   sock->Send((const byte*) &sendLen, sizeof(int));
   bytes = sock->Send((const byte*) encoded.c_str(), sendLen);

   return originalBytes;

}

int symRead( CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket * sock,
         char * buff, int len )
{
   string decoded = "", recovered = "", ack = "", recoveredPlusTemp = "";
    size_t bytes = 0;
    byte* tempBuf;
    const int flags = HashVerificationFilter::THROW_EXCEPTION
 	        | HashVerificationFilter::HASH_AT_END;

    int receiveLen;
    sock->Receive((byte*) &receiveLen, sizeof(int));
    tempBuf = new byte [receiveLen+1];
    memset(tempBuf, 0, receiveLen+1);
    bytes = sock->Receive(tempBuf, receiveLen);

    StringSource(tempBuf, receiveLen, true, new Base64Decoder(new StringSink(decoded)) // Base64Encoder
 	        );// StringSource

    delete [] tempBuf;
    // The StreamTransformationFilter removes
    //  padding as required.
    StringSource s(decoded, true,
 	        new StreamTransformationFilter(dAES, new StringSink(recovered)) // StreamTransformationFilter
 	                 );// StringSource

    cout << "Symmetric Recovered: " << recovered << endl;

    sock->Receive((byte*) &receiveLen, sizeof(int));
    tempBuf = new byte [receiveLen+1];
    memset(tempBuf, 0, receiveLen+1);
    bytes = sock->Receive(tempBuf, receiveLen);


    decoded = "";
    StringSource(tempBuf, receiveLen, true, new Base64Decoder(new StringSink(decoded)) // Base64Encoder
 	        );// StringSource

    recoveredPlusTemp = recovered + decoded;

    StringSource(recoveredPlusTemp, true, new HashVerificationFilter(cmac, NULL, flags)); // StringSource

    cout << "Verified message" << endl;

    memset(buff, 0, len);
    memcpy(buff, recovered.c_str(), recovered.length());
    delete [] tempBuf;
    return recovered.length();
}

void CheckFin( string fin )
{
   if(fin == "FIN")
   {
	  throw(new Exception(Exception::OTHER_ERROR, "Received FIN, exiting now"));
   }
}
