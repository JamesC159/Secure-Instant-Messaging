//This will be the server side program
#ifndef ServerSocket_h
#define ServerSocket_h

#include "Socket.h"

class ServerSocket : public Socket {
   
public:
   
   ServerSocket( const int port );
   ServerSocket(){}
   virtual ~ServerSocket(){}
   
   void accept( ServerSocket & );
   

};
#endif
