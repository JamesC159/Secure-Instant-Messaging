#ifndef Server_h
#define Server_h

#include "ServerSocket.h"

#include <string>

const int DEFAULT_PORT = 6161;

class Server {
   
private:
   
   Server( const Server & );
   void operator=( const Server & );
   
protected:
   
   Socket * listenSock;
   
public:
   
   Server( const int portno = DEFAULT_PORT ) {
      listenSock = new ServerSocket( portno );
   }
   
   ~Server() {
      delete listenSock;
   }
   
   Socket * getSocket() const { return listenSock; }
};
#endif
