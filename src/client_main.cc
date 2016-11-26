#include <networking.h>
#include <clienthelp.h>
#include <csignal>
#include <errno.h>

#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::getline;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;
using std::ostringstream;
using std::istringstream;

const string FIN_STR = "FIN"; // These flags can be whatever we want them to be.
const string SYN_STR = "SYN";
const string RST_STR = "RST";
Socket sockServer;

void
SignalHandler(int signum)
{
  cout << "Abort signal (" << signum << ") received.\n";

  try
  {
      sockServer.ShutDown(SHUT_RDWR);
  }
  catch(Exception& e)
  {
      cerr << e.what() << endl;
  }

  exit(signum);

}

/******************************************************************************
 *                         MAIN FUNCTION
 *****************************************************************************/
int
main(int argc, char ** argv)
{
  signal(SIGTERM, SignalHandler);

  // Get listening socket and address
  /*
   int serverSock = socket (AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in sAddr;

   bzero ( (void *) &sAddr, sizeof(sAddr) );
   sAddr.sin_family = ( AF_INET );
   sAddr.sin_addr.s_addr = htonl ( INADDR_ANY );
   sAddr.sin_port = htons ( 0 );

   if(bind( serverSock, (struct sockaddr *) & sAddr, sizeof(sAddr)))
   {
   fprintf(stderr, "Node %d failed to bind listening socket.\n", c->nodeID);
   exit ( -1 );
   }
   else
   {
   fprintf(stdout,
   "Node %d bound listening socket successfully.\n",
   c->nodeID);
   }

   listen ( serverSock, 20);
   */
  // connect to server

  try
    {
      AutoSeededRandomPool rng;
      RSA::PublicKey serverKey;
      LoadPublicKey("rsa-public.key", serverKey);
      if (!serverKey.Validate(rng, 3))
        {
          cerr << "Failed to load public RSA key";
          return -1;
        }

      sockServer.Create();
      sockServer.Connect("localhost", port);

      // login - agree on session key with server
      // get buddy list from server
      // wait on user for request to comm with another client
      // mutual authentication between requested client
      // derive encryption/authentication keys from session key
      // let user talk (and display messages from other user)
      // disconnect


      string sendBuf, recBuf, temp, recovered;
      Integer m, c, r;

      cout << "Enter your username: ";
      if (!getline(cin, sendBuf))
        {
          cout << "Failed" << endl;
        }
      sendMsg(serverKey, sockServer, sendBuf);
      recovered = recoverMsg(serverKey, sockServer);

      if (recovered.compare(FIN_STR) == 0)
        {
          cout << "Since receiving the FIN ACK from server, we are exiting"
              << endl;
          sockServer.ShutDown(SHUT_RDWR);
        }

      //Parse nonce and salt reply
      istringstream is(recovered);
      Integer nonce, salt;
      is >> nonce >> salt;
      cout << "Nonce: " << nonce << endl << "Salt: " << salt << endl;
      is.str(string());

      //Reply with username, hash of pw, and nonce
      const byte* uname = (const byte *) sendBuf.c_str();
      byte* s;
      string ptemp = "1";
      byte* p = (byte*)ptemp.c_str();
      byte digest[SHA256::DIGESTSIZE];
      SHA256 hash;

      ostringstream os;
      os << salt;
      temp = os.str();
      s = (byte*)temp.c_str();
      cout << "Salt Encoded: " << s << endl;
      hash.Update(p, sizeof(p));        //hash password first
      hash.Update(s, sizeof(s));        //then hash slat
      hash.Final(digest);               //final result
      cout << "Sending Hash Stuff" << endl;
      os.str("");
      os << uname << "~" << digest << "~" << nonce;
      cout << "Final String to be Sent: " <<  os.str() << endl;
      sendBuf = os.str();
      os.str("");
      sendMsg(serverKey, sockServer, sendBuf);
      sockServer.ShutDown(SHUT_RDWR);
    }
  catch (Exception& e)
    {
      cout << "Exception caught: " << e.what() << endl;
      sockServer.ShutDown(SHUT_RDWR);
    }
}
