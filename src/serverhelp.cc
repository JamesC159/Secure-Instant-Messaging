/*
 * serverhelp.cc
 *
 *  Created on: Nov 26, 2016
 *      Author: root
 */

#include "serverhelp.h"

string
recoverMsg(struct ThreadData * tdata)
{
  string recovered, recBuf;
  ostringstream ss;
  Integer c, r, m;
  AutoSeededRandomPool rng;
  byte byteBuf[MAX_BUF];
  memset(byteBuf, NULL, sizeof(byteBuf));

  try
    {
      // Retrieve message from socket
      cout << "Waiting to receive a message from client " << tdata->tid << endl;

      tdata->sockSource.Receive(byteBuf, sizeof(byteBuf));

          //Convert message to a string
          ss << byteBuf;
          recBuf = ss.str();
          ss.str(string());
          cout << "Cipher Received: " << recBuf << endl;

          //Convert the string to an Integer so we can calculate the inverse
          c = Integer(recBuf.c_str());
          r = tdata->privateKey.CalculateInverse(rng, c);

          // Recover the original message
          size_t req = r.MinEncodedSize();
          recovered.resize(req);
          r.Encode((byte *) recovered.data(), recovered.size());
          cout << "Recovered: " << recovered << endl;

    }
  catch (Exception& e)
    {
      cerr << "caught Exception..." << endl;
      cerr << e.what() << endl;
      tdata->sockSource.ShutDown(SHUT_RDWR);
    }
  return recovered;
}

void
sendMsg(string sendBuf, struct ThreadData * tdata)
{
  AutoSeededRandomPool rng;
  Integer m, c, r;
  ostringstream ss;

  try
    {
      // Encode the message as an Integer
      m = Integer((const byte *) sendBuf.c_str(), sendBuf.size());

      //Encrypt
      c = tdata->privateKey.CalculateInverse(rng, m);

      //Turn the encrypted value into a string
      ss << c;
      sendBuf = ss.str();
      ss.str(string());
      cout << "Cipher Sent: " << sendBuf << endl;

      tdata->sockSource.Send((const byte *) sendBuf.c_str(), sendBuf.size());

    }
  catch (Exception& e)
    {
      cerr << "caught Exception..." << endl;
      cerr << e.what() << endl;
      tdata->sockSource.ShutDown(SHUT_RDWR);
    }
}
