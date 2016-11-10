#include "Server.h"
#include "ServerSocket.h"

#include <iostream>

int main( int argc, char ** argv ) {
   
   if( argc < 2 ) {
      std::cout << "Usage: ./server portno\n";
      return 0;
   }
   
   try {
      Server server( atoi( argv[1] ) );
      
      printf( "Server creation successful\n" );
      printf( "Server listening port number: %d\n",
             server.getSocket() -> getPort() );
      printf( "Server IP Address: %s\n",
             server.getSocket() -> getAddr() );
      
      // Accept client connections

   }
   catch ( SocketException & e ) {
      std::cout << "Exception was caught: " << e.description() << std::endl;
   }
   
   return 0;
}
