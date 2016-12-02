/*
 * clienthelp.cc
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#include "clienthelp.h"
#include <sys/socket.h>

string recoverMsg( RSA::PublicKey serverKey, Socket& sockServer )
{
	string recovered, recBuf;
	ostringstream ss;
	Integer c, r, m;
	AutoSeededRandomPool rng;
	byte byteBuf[ MAX_BUF ];
	memset(byteBuf, NULL, sizeof(byteBuf));

	try
	{
		// Retrieve message from socket
		cout << "Waiting for reply from server..." << endl;
		sockServer.Receive(byteBuf, sizeof(byteBuf));
		cout << endl << "Cipher Received: " << byteBuf << endl;

		// Convert message to a string
		ss << byteBuf;
		recBuf = ss.str();
		ss.str(string());

		//Convert the string to an Integer so we can calculate the inverse
		c = Integer(recBuf.c_str());
		r = serverKey.ApplyFunction(c);
		cout << "r: " << r << endl;

		// Recover the original message
		size_t req = r.MinEncodedSize();
		recovered.resize(req);
		r.Encode((byte *) recovered.data(), recovered.size());
		cout << "Recovered: " << recovered << endl;

	}
	catch ( Exception& e )
	{
		cerr << "caught Exception..." << endl;
		cerr << e.what() << endl;
		sockServer.ShutDown(SHUT_RDWR);
		return "";
	}

	return recovered;
}

void sendMsg( RSA::PublicKey serverKey, Socket& sockServer, string sendBuf )
{
	AutoSeededRandomPool rng;
	Integer m, c, r;
	ostringstream ss;

	try
	{
		// Encode the message as an Integer
		m = Integer((const byte *) sendBuf.c_str(), sendBuf.size());

		//Encrypt
		c = serverKey.ApplyFunction(m);

		//Turn the encrypted value into a string
		ss << c;
		sendBuf = ss.str();
		ss.str(string());
		cout << "Cipher Sent: " << sendBuf << endl;

		sockServer.Send((const byte *) sendBuf.c_str(), sendBuf.size());

	}
	catch ( Exception& e )
	{
		cerr << "caught Exception..." << endl;
		cerr << e.what() << endl;
		sockServer.ShutDown(SHUT_RDWR);
	}
}
