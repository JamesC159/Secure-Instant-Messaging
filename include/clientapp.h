#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <thread>
#include <mutex>
#include <sstream>
#include <iostream>
#include <string>


#include <cryptopp/socketft.h>
using CryptoPP::Socket;


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "getch.h"

extern std::string ownName;
extern std::string otherName;



void startTalking(Socket * sock);
void sockListener(Socket * sock);


// from client_main.cc
extern Socket cliSock;

#endif
