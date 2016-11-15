#include <networking.h>



int main(int argc, char ** argv ) {
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
       fprintf(stdout, "Node %d bound listening socket successfully.\n", c->nodeID);
   }

   listen ( serverSock, 20);
   */
   // connect to server
   int cliSock = connectToHost("localhost", 12345);
   if (cliSock < 0)
   {
      perror("Failed to connect to server");
      return -1;
   }
   char * buf = "Hello";
   write(cliSock, buf, strlen(buf));
   char buffer [1024];
   memset(buffer, '\0', sizeof(buffer));
   read(cliSock, buffer, sizeof(buffer));
   fprintf(stdout, "Got back: %s\n", buffer);
   close(cliSock);

   // login
   // get buddy list
   // wait on user for request
   // connect to other client
   // let user talk (and display messages from other user)
   // disconnect
}
