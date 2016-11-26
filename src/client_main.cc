#include <networking.h>
#include <errno.h>
#include <iostream>
using std::cout;
using std::cin;
using std::endl;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;
using std::ostringstream;
using std::istringstream;

const string FIN_STR = "FIN"; // These flags can be whatever we want them to be.
const string SYN_STR = "SYN";
const string RST_STR = "RST";

void
sendMsg(RSA::PublicKey, Socket&, string);
string
recoverMsg(RSA::PublicKey, Socket&);

/******************************************************************************
 *                         MAIN FUNCTION
 *****************************************************************************/
int
main(int argc, char ** argv)
{

  int portno = -1;

  if (argc < 2)
    {
      fprintf(stderr, "Usage: client <portno>\n");
      return -1;
    }

  portno = validatePort(argv[1]);
  if (portno == -1)
    {
      if (errno)
        {
          perror("ERROR failed to convert port number argument to integer");
        }
      else
        {
          fprintf(stderr, "ERROR invalid port number\n");
        }

      return 1;
    }
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

      Socket sockServer;
      sockServer.Create();
      sockServer.Connect("localhost", portno);

      // login - agree on session key with server
      // get buddy list from server
      // wait on user for request to comm with another client
      // mutual authentication between requested client
      // derive encryption/authentication keys from session key
      // let user talk (and display messages from other user)
      // disconnect

      bool fin = false;
      while (!fin)
        {
          string sendBuf, recBuf, temp, recovered;
          ostringstream ss;
          Integer m, c, r;

          cout << "Enter secret to send to server: ";
          if (!getline(cin, sendBuf))
            {
              cout << "Failed" << endl;
            }
          if (sendBuf.compare(FIN_STR) == 0)
            {
              fin = true;
            }

          cout << "fin value: " << fin << endl;
          sendMsg(serverKey, sockServer, sendBuf);
          recovered = recoverMsg(serverKey, sockServer);

          if (recovered.compare(FIN_STR) == 0)
            {
              cout << "Since receiving the FIN ACK from server, we are exiting"
                  << endl;
              sockServer.CloseSocket();
              fin = true;
            }

        }
    }
  catch (Exception& e)
    {
      cout << "Exception caught: " << e.what() << endl;
    }
}
