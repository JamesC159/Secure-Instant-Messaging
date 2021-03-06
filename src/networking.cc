#include <networking.h>
#include <errno.h>

/******************************************************************************
 * FUNCTION:      
 * DESCRIPTION:   
 * PARAMETERS:    
 * RETURN: 
 * NOTES:        
 *****************************************************************************/
int connectToHost(const char * hostName, int port)
{
   int cliSock = socket(AF_INET, SOCK_STREAM, 0);
   struct sockaddr_in serverAddr;
   bzero((void *) &serverAddr, sizeof(serverAddr));
   struct hostent * he;
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(port);
   he = gethostbyname(hostName);      // This should probably be gethostbyaddr()
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
/******************************************************************************
 * FUNCTION:      
 * DESCRIPTION:   
 * PARAMETERS:    
 * RETURN:
 * NOTES:         
 *****************************************************************************/
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
/******************************************************************************
 * FUNCTION:      
 * DESCRIPTION:   
 * PARAMETERS:    
 * RETURN:         
 * NOTES:
 *****************************************************************************/
bool
createSocket( int & sockDesc )
{
   
   sockDesc = socket( AF_INET,
                      SOCK_STREAM,
                      0 );
   
   if ( sockDesc < 0 )
   {
      return false;
      
   }
   
   return true;
}
/******************************************************************************
 * FUNCTION:      
 * DESCRIPTION:   
 * PARAMETERS:    
 * RETURN:         
 * NOTES:
 *****************************************************************************/
bool
bindSocket( int & sockDesc,
            struct sockaddr_in & sockAddr )
{
   
   if ( bind( sockDesc,
             (struct sockaddr *) & sockAddr,
             (socklen_t)sizeof( sockAddr ) ) < 0 )
   {
      
      return false;
   }
   
   return true;
}
/******************************************************************************
 * FUNCTION:      
 * DESCRIPTION:   
 * PARAMETERS:    
 * RETURN:         
 * NOTES:
 *****************************************************************************/
bool
listenSocket( int & sockDesc )
{
   
   if ( listen ( sockDesc, MAX_CONN ) < 0 )
   {
      return false;
   }
   
   return true;
}
/******************************************************************************
 * FUNCTION:      
 * DESCRIPTION:   
 * PARAMETERS:    
 * RETURN:         
 * NOTES:
 *****************************************************************************/
bool
acceptSocket( int & servDesc,
              int & cliDesc,
              struct sockaddr_in & cliAddr,
              socklen_t & cliLen )
{
      
   cliDesc = accept( servDesc,
                    (struct sockaddr *) & cliAddr,
                    & cliLen );
   
   if ( cliDesc < 0 )
   {
      return false;
   }
   
   return true;
}
/******************************************************************************
 * FUNCTION:      
 * DESCRIPTION:   
 * PARAMETERS:    
 * RETURN:         
 * NOTES:
 *****************************************************************************/
int
validatePort( const char * portStr )
{
   int portNo = (int)strtol( portStr, NULL, 10 );

   if (errno || portNo <= 0 || portNo > 65535 )
   {
      return -1;
   }
   
   return portNo;
}
