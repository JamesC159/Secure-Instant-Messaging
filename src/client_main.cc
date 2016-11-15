#include <networking.h>
#include <errno.h>

int main(int argc, char ** argv ) {
   
   int portno = -1;
   bool FIN = false;
   
   if (argc < 2 )
   {
      fprintf( stderr, "Usage: client <portno>\n");
      return -1;
   }
   
   portno = validatePort( argv[1] );
   if ( portno == -1 )
   {
      if ( errno )
      {
         perror( "ERROR failed to convert port number argument to integer" );
      }
      else
      {
         fprintf( stderr, "ERROR invalid port number\n" );
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
   int cliSock = connectToHost("localhost", portno);
   if (cliSock < 0)
   {
      perror("Failed to connect to server");
      return -1;
   }

   while( ! FIN )
   {
      char * buf = (char *)malloc( 8192*sizeof( char ) );
      size_t size = 8192*sizeof(char);
      if( buf == NULL )
      {
         fprintf( stderr, "ERROR failed to allocate space for the buffer\n" );
         return -1;
      }

      if( getline(&buf, &size, stdin) < 0 )
      {
         fprintf( stderr, "ERROR failed to read input from stdin\n" );
      }

      if( strcmp( buf, FIN_STR ) == 0 )
      {
         FIN = true;
      }

      write(cliSock, buf, strlen(buf));
      char buffer [8192];
      memset(buffer, '\0', sizeof(buffer));
      read(cliSock, buffer, sizeof(buffer));
      fprintf(stdout, "Got back: %s\n", buffer);
      free( buf );
   }

   close(cliSock);

   // login
   // get buddy list
   // wait on user for request
   // connect to other client
   // let user talk (and display messages from other user)
   // disconnect
}
