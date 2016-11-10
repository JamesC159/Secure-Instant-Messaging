//This will be the server side program
#include "ServerSocket.h"

ServerSocket::ServerSocket( const int port ) {
   
   if ( ! Socket::create() ) {
      throw SocketException( " Socket Exception: Failed to create server socket"
                            " descriptor." );
   }
   
   if ( ! Socket::bind( port ) ) {
      
      throw SocketException( "Socket Exception: Failed to bind to the port." );
   }
   
   if ( ! Socket::listen() ) {
      
      throw SocketException( "Socket Exception: Failed to listen on the socket." );
   }
}

void ServerSocket::accept( ServerSocket & clientSock ) {
   
   if ( ! Socket::accept( clientSock ) ) {
      throw SocketException( "Socket Exception: Failed to accept the socket" );
   }
}
