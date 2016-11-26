#include <networking.h>
#include <serverhelp.h>
#include <buddy.h>
#include <csignal>

struct clientDB
{
  char * username;
  char * pwHash;
  int salt;
};

const int BUDDIES = 15;
const string FIN_STR = "FIN";
const string SYN_STR = "SYN";
const string RST_STR = "RST";
struct ThreadData * tdata;
Buddy* buddylist = new Buddy[BUDDIES];
Socket sockListen;
Socket sockSource;

void
processRequest(char *);

bool
authenticate(struct ThreadData *);

void *
clientWorker(void *);

void
SignalHandler(int signum)
{
  cout << "Abort signal (" << signum << ") received.\n";

  try
  {
      sockListen.ShutDown(SHUT_RDWR);
      sockSource.ShutDown(SHUT_RDWR);
  }
  catch(Exception& e)
  {
      cerr << e.what() << endl;
  }

  exit(signum);

}

/******************************************************************************
 *                            MAIN FUNCTION       
 *****************************************************************************/
int
main(int argc, char ** argv)
{

  pthread_t clientThread;
  int rc = 0;
  int tcount = 1;
  sockaddr_in clientaddr;
  socklen_t clientlen = sizeof(clientaddr);
  signal(SIGTERM, SignalHandler);

  try
    {
      sockListen.Create();
      sockSource.Create();
      sockListen.Bind(port);
      sockListen.Listen();

      AutoSeededRandomPool rng;

      RSA::PrivateKey privateKey;
      RSA::PublicKey publicKey;
      LoadPrivateKey("rsa-private.key", privateKey);
      if (!privateKey.Validate(rng, 3))
        {
          cerr << "Failed to load private RSA key";
          return -1;
        }
      LoadPublicKey("rsa-public.key", publicKey);
      if (!publicKey.Validate(rng, 3))
        {
          cerr << "Failed to load public RSA key";
          return -1;
        }

      // Since we know who the buddies are, we are going to initialize them here.
      for (int i = 0; i < BUDDIES; i++)
        {
          buddylist[i].SetUname("Billy" + i);
          buddylist[i].SetPW("i");
          buddylist[i].SetAvailable(true);
        }

      // Start client listen-accept phase.
      while (true)
        {

          sockListen.Accept(sockSource, (sockaddr*) &clientaddr, &clientlen);
          tdata = new struct ThreadData;
          tdata->tid = tcount;
          tdata->clientaddr = clientaddr;
          tdata->clientlen = clientlen;
          tdata->sockListen = sockListen;
          tdata->sockSource = sockSource;
          tdata->publicKey = publicKey;
          tdata->privateKey = privateKey;

          rc
              = pthread_create(&clientThread, NULL, clientWorker,
                  (void *) tdata);
          if (rc)
            {
              fprintf(stderr, "ERROR creating client thread : %d\n", rc);
              return 1;
            }
          tcount++;
        }

      delete[] buddylist;
      sockListen.ShutDown(SHUT_RDWR);
    }
  catch (CryptoPP::Exception& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
    }

  return 0;

}

/******************************************************************************
 * FUNCTION:      clientWorker    
 * DESCRIPTION:   Thread worker function for accepted client socket 
 *                connections.
 * PARAMETERS:    void * in - ThreadData structure
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void *
clientWorker(void * in)
{

  bool fin = false;
  bool authenticated = false; // Flag for client authentication recognition.
  string buffer;

  struct ThreadData * tdata; // Client thread data structure.

  tdata = (struct ThreadData *) in;
  cout << "Client thread " << tdata->tid << " Starting..." << endl;

  // First we must authenticate the client. This involves establishing the
  // session key between the client and the server.
  if (!authenticated)
    {
      fin = authenticate(tdata);
      if (!fin)
        {
          cout << "Client did not authenticate" << endl;
          sockSource.ShutDown(SHUT_RDWR);
          return (void*) -1;
        }
      else
        {
          authenticated = true;
          cout << "Client successfully authenticated" << endl;
        }

    }

  cout << "Beginning session with client " << tdata->tid << "..." << endl;
  fin = false;
  while (!fin)
    {
      buffer = recoverMsg(tdata);
      cout << "Enter ACK to client " << tdata->tid << ": ";
      if (!getline(cin, buffer))
        {
          cout << "Failed" << endl;
        }
      sendMsg(buffer, tdata);
    }

  sockSource.ShutDown(SHUT_RDWR);
  return (void*) 0;

}

/******************************************************************************
 * FUNCTION:      processRequest  
 * DESCRIPTION:   Determine client requests to server.
 * PARAMETERS:    char * request 
 *                   - The client request to the server
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void
processRequest(char * request)
{
  // Check if client session request.
  //    Agree on session key for client-client comm.
  //    Get ticket to the requested client and send it.
  // Check if buddy list request.
  // Check if client wants to close the connection.

}

/******************************************************************************
 * FUNCTION:      authenticate   
 * DESCRIPTION:   Authenticates and establishes Diffie-Hellman session key
 *                between a client and the server.   
 * PARAMETERS:    struct ThreadData ** tdata
 *                   - socket descriptor and thread id for the client.
 * RETURN:        true if client successfully authenticated.
 *                false if client authentication was a failure.
 * NOTES:       
 *****************************************************************************/
bool
authenticate(struct ThreadData * tdata)
{
  try
    {
      string recovered = recoverMsg(tdata); //Get message from client

      string sendBuf;
      AutoSeededRandomPool rng;
      Integer nonce, salt;
      bool foundBuddy = false;

      for (int i = 0; i < BUDDIES; i++)
        {
          if (buddylist[i].GetUname().compare(recovered) == 0)
            {
              ostringstream os;
              nonce = Integer(rng, 64);
              salt = Integer(rng, 64);
              buddylist[i].SetSalt(salt);
              os << nonce << " " << salt;
              sendBuf = os.str();
              os.str(string());
              foundBuddy = true;
              cout << "Sending ACK back to client" << endl;
              sendMsg(sendBuf, tdata);
            }
        }
      if (!foundBuddy)
        {
          cout << "Buddy not found" << endl;
          sendBuf = "FIN";
          sendMsg(sendBuf, tdata);
          return false;
        }

      byte* s;
      ostringstream os("");
      os << salt;
      string temp = os.str();
      cout << "Salt Encoded: " << temp << endl;
      s = (byte*) temp.c_str();
      os.str("");

      recovered = recoverMsg(tdata);
      istringstream is(recovered);
      SHA256 hash;
      string uname, digest, nstr;
      string tnonce;

      std::getline(is, uname, '~');
      std::getline(is, digest, '~');
      std::getline(is, tnonce, '~');
      nonce = Integer(tnonce.c_str());
      cout << "Server received: " << endl
          << uname << endl
          << digest << endl
          << nonce << endl;

      string ptemp = "1";
      byte* p = (byte*) ptemp.c_str();
      hash.Update(p, sizeof(p));
      hash.Update(s, sizeof(s));
      if (!hash.Verify((byte *) digest.c_str()))
        {
          throw "Failed to verify hash";
        }
      else
        {
          cout << "Message Digest successfully verified" << endl;
        }
      return true;
    }
  catch (CryptoPP::Exception& e)
    {
      cerr << "caught Exception..." << endl;
      cerr << e.what() << endl;
      return false;
    }
  catch (const char* e)
    {
      cerr << "caught Exception..." << endl;
      cerr << e << endl;
      return false;
    }
}

