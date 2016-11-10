#ifndef Socket_h
#define Socket_h

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

const int MAX_CONN = 100;

class SocketException {
   
private:
   
   std::string userMsg;
   
public:
   
   SocketException( const std::string & msg ) : userMsg ( msg ) {}
   ~SocketException(){}
   
   std::string description() { return userMsg; }
}; // End class SocketException

class Socket {
   
private:
   
   //Copy constructor and assignment operator should not be accessible for a socket
   Socket( const Socket & );
   void operator=( const Socket & );
   
protected:
   
   int sockDesc;
   struct sockaddr_in sockAddr;
   
public:
   
   Socket();
   virtual ~Socket();
   
   // Socket wrappers
   bool create();
   bool bind( const int );
   bool listen() const;
   bool accept( Socket & );
   bool connect( std::string, const int );
   
   // Test methods
   bool isOpen() const { return sockDesc != -1; }
   
   // Getters
   int getPort() const { return ntohs( sockAddr.sin_port ); }
   char * getAddr() const { return inet_ntoa( sockAddr.sin_addr ); }
   
   // Mutators
   
};

#endif
