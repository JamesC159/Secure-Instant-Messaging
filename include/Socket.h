#ifndef SOCKET_H
#define SOCKET_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Socket
{
   private:
   	int socketfd;
   	int portno;
   	socklen_t len;
   	struct sockaddr_in addr;
   public:
      Socket(){}
      ~Socket(){}
      
};
#endif
