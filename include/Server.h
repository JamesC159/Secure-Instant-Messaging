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
   Socket * newSock;
   
public:
   
   Server( const int portno = DEFAULT_PORT ) {
      listenSock = new ServerSocket( portno );
      newSock = new ServerSocket();
   }
   
   ~Server() {
      delete listenSock;
      delete newSock;
   }
   
   Socket * getListener() const { return listenSock; }
   Socket * getAcceptee() const { return newSock; }
};
#endif
