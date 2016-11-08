#include "Client.h"
#include "Server.h"
#include "Socket.h"

#include <iostream>

const int MAX_CLIENTS = 100;

int main( int argc, char** argv )
{
	Server server;
	Client* clients = new Client[MAX_CLIENTS];
	
	delete [] (clients);
	return 0;
}
