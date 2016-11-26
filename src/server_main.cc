/*
 * Assumptions:   (1) The buddy list is already known in advance
 *                (2) The username and passwords of clients are already known
 *                    in advance.
 * TODO:
 */

#include <networking.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>
using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::flush;

#include <string>
using std::string;

#include <sstream>
using std::stringstream;
using std::istringstream;
using std::ostringstream;

struct clientDB
{
  char * username;
  char * pwHash;
  int salt;
};

void
checkReadWrite(int);

void
processRequest(char *);

void
readFromClient(int &, string);

void
writeToClient(int &, string);

bool
authenticate(struct clientThreadData *);

void *
clientWorker(void *);

Socket sockListen;
Socket sockSource;
const string FIN_STR = "FIN"; // These flags can be whatever we want them to be.
const string SYN_STR = "SYN";
const string RST_STR = "RST";
struct clientThreadData * cData;

/******************************************************************************
 *                            MAIN FUNCTION       
 *****************************************************************************/
int
main(int argc, char ** argv)
{

  pthread_t clientThread; // Thread to spawn client workers.
  int portNo = -1; // Port number server is binded to.
  int rc = 0; // pthread_create() return value.
  int tcount = 1; // Thread id counter for each client.

  // Make sure the port number was provided in the command line arguments.
  if (argc < 2)
    {
      fprintf(stderr, "Usage: ./server <portno>\n");
      return 0;
    }

  // Validate and store the port number provided by the user.
  portNo = validatePort(argv[1]);

  if (portNo == -1)
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

  try
    {

      sockListen.Create();
      sockSource.Create();
      sockListen.Bind(portNo);
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

      // Start client listen-accept phase.
      while (true)
        {

          sockListen.Accept(sockSource);

          // Create a new clientThreadData structure.
          cData = new struct clientThreadData;
          cData -> tid = tcount;
          cData -> privateKey = privateKey;
          cData -> publicKey = publicKey;

          // Spawn a worker thread for the connecting client.
          rc
              = pthread_create(&clientThread, NULL, clientWorker,
                  (void *) cData);
          if (rc)
            {
              fprintf(stderr, "ERROR creating client thread : %d\n", rc);
              return 1;
            }

          tcount++;
        }

      sockListen.CloseSocket();
    }
  catch (CryptoPP::Exception& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
    }

  return 0;

}

/******************************************************************************
 * FUNCTION:      checkReadWrite  
 * DESCRIPTION:   Validates socket reads and writes
 * PARAMETERS:    int bytes
 *                   - number of bytes read or written to a socket descriptor.
 * RETURN:        None  
 * NOTES:      
 *****************************************************************************/
void
checkReadWrite(int bytes)
{
  // Reminder - stack buffer overflows. We need to do something about writing
  // too much or reading too much.
  if (bytes < 0 || bytes > MAX_BUF)
    {
      perror("ERROR failed reading/writing to socket ");
    }
}

/******************************************************************************
 * FUNCTION:      readFromClient  
 * DESCRIPTION:   reads bytes written to socket descriptor from client
 * PARAMETERS:    char ** buffer
 *                   - message buffer
 *                struct clientThreadData ** cData
 *                   - structure containing client socket/thread info
 * RETURN:        None
 * NOTES:      
 *****************************************************************************/
void
readFromClient(int & socket, string buffer)
{
  int bytes = 0;

  bytes = read(socket, (char*) buffer.c_str(), MAX_BUF);
  checkReadWrite(bytes);

}

/******************************************************************************
 * FUNCTION:      readFromClient  
 * DESCRIPTION:   reads bytes written to socket descriptor from client
 * PARAMETERS:    char ** buffer
 *                   - message buffer
 *                struct clientThreadData ** cData
 *                   - structure containing client socket/thread info
 * RETURN:        None
 * NOTES:      
 *****************************************************************************/
void
writeToClient(int & socket, string msg)
{
  int bytes = 0;

  bytes = write(socket, (char*) msg.c_str(), MAX_BUF);
  checkReadWrite(bytes);

}

/******************************************************************************
 * FUNCTION:      clientWorker    
 * DESCRIPTION:   Thread worker function for accepted client socket 
 *                connections.
 * PARAMETERS:    void * in - clientThreadData structure
 * RETURN:        None
 * NOTES:
 *****************************************************************************/
void *
clientWorker(void * in)
{

  bool fin = false;
  bool authenticated = false; // Flag for client authentication recognition.
  string buffer;

  struct clientThreadData * cData; // Client thread data structure.

  cData = (struct clientThreadData *) in;
  printf("Client thread %d starting\n", cData -> tid);

  // First we must authenticate the client. This involves establishing the
  // session key between the client and the server.

  if (!authenticated)
    {
      fin = authenticate(cData);
      if (!fin)
        {
          cout << "Client did not authenticate" << endl;
          sockSource.CloseSocket();
          return (void*) -1;
        }
      else
        {
          authenticated = true;
          cout << "Client successfully authenticated" << endl;
        }

    }

  fin = false;
  while (!fin)
    {
      if (!getline(cin, buffer))
        {
          cout << "Failed" << endl;
        }
      buffer = recoverMsg(sockSource, cData);
      buffer = "My Reply";
      sendMsg(sockSource, buffer, cData);
    }

  sockSource.CloseSocket();
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
 * PARAMETERS:    struct clientThreadData ** cData 
 *                   - socket descriptor and thread id for the client.
 * RETURN:        true if client successfully authenticated.
 *                false if client authentication was a failure.
 * NOTES:       
 *****************************************************************************/
bool
authenticate(struct clientThreadData * cData)
{
  try
    {
      string recovered = recoverMsg(sockSource, cData);

      // Here we look up the username etc.
      string sendBuf;
      if (recovered.compare(FIN_STR) == 0)
        {
          sendBuf = "FIN";
          cout << "Client requested to close the connection" << endl;
          sendMsg(sockSource, sendBuf, cData);
          return false;
        }
      else
        {
          if (!getline(cin, sendBuf))
            {
              cout << "Failed" << endl;
            }

          cout << "Sending authentication ACK back to client" << endl;
          sendMsg(sockSource, sendBuf, cData);

        }
    }
  catch (CryptoPP::Exception& e)
    {
      cerr << "caught Exception..." << endl;
      cerr << e.what() << endl;
    }

  return true;
}

