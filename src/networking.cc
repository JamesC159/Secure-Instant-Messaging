#include <networking.h>

int connectToHost(const char * hostName, int port)
{
   int cliSock = socket(AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in serverAddr;
   bzero((void *) &serverAddr, sizeof(serverAddr));
   struct hostent * he;
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(port);
   he = gethostbyname(hostName);
   if (he == NULL)
   {
      fprintf(stderr, "Failed to resolve hostname\n");
      return -1;
   }
   memcpy(&serverAddr.sin_addr, he->h_addr_list[0], he->h_length);
   if (connect(cliSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
   {
      perror("Failed to connect to remote host");
      return -1;
   }
   return cliSock;
}


// Hex 07 is end of application message
char * readFromSocket(int sock, char * & buffer)
{
   if (buffer == NULL)
   {
      buffer = (char *) calloc(1024, sizeof(char));
      if (buffer == NULL)
      {
         perror("Failed to allocate buffer space in readFromSocket");
	 return NULL;
      }
   }




   return NULL;
}
