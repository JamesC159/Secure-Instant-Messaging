//This will be the server side program
#ifndef SERVER_H
#define SERVER_H

#include "Socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

class Server
{
   private:
   public:
   Server(){};
   ~Server(){};
   void accept( Socket & );
};
#endif
