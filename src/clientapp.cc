#include <clientapp.h>
#include <clienthelp.h>


Socket connectedSock;
std::string ownName;
std::string otherName;
bool done;
int charsRead;
std::stringstream ss;
std::mutex screenLock;
CBC_Mode < AES >::Encryption commonE;
CBC_Mode < AES >::Decryption commonD;
CMAC< AES > commonCmac;

void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void startTalking(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket *sock)
{
   done = false;
   charsRead = 0;
   ss.clear();
   ss.str("");
   std::thread lthread(sockListener, eAES, dAES, cmac, sock);
   while(true)
   {
      char c = getch();
      screenLock.lock();
      if (done)
      {
         charsRead = 0;
	 ss.str("");
	 printf("\n");
	 break;
      }
      if (c == '\n')
      {
	 symWrite(eAES, cmac, sock, ss.str().c_str(), charsRead);
	 printf("\r%s\r", std::string(charsRead, ' ').c_str());
	 printf("%s: %s\n", ownName.c_str(), ss.str().c_str());
	 if (ss.str() == "EXIT")
	 {
	    done = true;
            charsRead = 0;
	    ss.str("");
	    break;
	 }
         charsRead = 0;
	 ss.str("");
      }
      else
      {
         printf("%c", c);
         charsRead++;
         ss << c;
      }
      screenLock.unlock();
   }
   screenLock.unlock();
   lthread.join();
}

void sockListener(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket *sock)
{
   char buff [1024];
   memset(buff, '\0', sizeof(buff));
   while (true)
   {
      int cRead = symRead(dAES, cmac, sock, buff, sizeof(buff)-1);
      if (cRead > 0)
      {
         buff[cRead] = '\0';
	 screenLock.lock();
	 printf("\r%s\r", std::string(charsRead, ' ').c_str());
	 printf("%s: %s\n", otherName.c_str(), buff);
	 if (std::string(buff) == "EXIT")
	 {
	    if (!done)
	    {
	       symWrite(eAES, cmac, sock, buff, strlen(buff));
	       printf("Session has ended, press a key to continue...");
	       done = true;
	    }
	    screenLock.unlock();
	    break;
	 }
	 printf("%s", ss.str().c_str());
	 flush(std::cout);
	 screenLock.unlock();
	 memset(buff, '\0', sizeof(buff));
      }
      else if (cRead < 0)
      {
         fprintf(stderr, "ERROR: read returned with < 0\n");
	 return;
      }
   }
}

void connReqHdlr(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket *sock)
{
   char readBuff [5000];
   done = false;
   connectedSock.Create();
   std::thread lthread(incomingRequestHandler, dAES);
   int promptLen = charsRead = strlen("Who would you like to talk to?: ");
   ss.clear();
   ss.str("");
   ss << "Who would you like to talk to?: ";
   cout << "Who would you like to talk to?: ";
   while(true)
   {
      char c = getch();
      screenLock.lock();
      if (done)
      {
         charsRead = 0;
	 ss.str("");
	 printf("\n");
	 break;
      }
      printf("%c", c);
      if (c == '\n')
      {
         symWrite(eAES, cmac, sock, ss.str().substr(promptLen).c_str(), ss.str().substr(promptLen).length());
         memset(readBuff, 0, sizeof(readBuff));
         symRead(dAES, cmac, sock, readBuff, sizeof(readBuff));
         done = true;
         charsRead = 0;
	 ss.str("");
	 CheckFin(string(readBuff));
         // Any other work with server and other client to connect
         //  includes setting connectedSock
         ss << readBuff;
	 cout << "Got response: " << ss.str() << endl << "End of response" << endl;
         string oName, ip, port, encAesKey, enciv, enccmac, ticket;
         std::vector<string> elims = split(ss.str(), ' ');
	 cout << "elims count: " << elims.size() << endl;
         oName = elims[0];
         ip = elims[1];
         port = elims[2];
         encAesKey = elims[3];
         enciv = elims[4];
         enccmac = elims[5];
         ticket = elims[6];
	 cout << "ticket: " << ticket << endl;
         connectedSock.Connect(ip.c_str(), std::stoi(port));
	 int len2send = ticket.size();
	 connectedSock.Send((const byte *) &len2send, sizeof(int));
         connectedSock.Send((const byte *) ticket.c_str(), len2send);
         SecByteBlock cmac_key(AES::DEFAULT_KEYLENGTH);
         SecByteBlock common_key(AES::DEFAULT_KEYLENGTH);
         byte iv[ AES::BLOCKSIZE ];
         ArraySource((const byte *) encAesKey.c_str(), encAesKey.size(), true, new Base64Decoder(new ArraySink(common_key, sizeof(common_key))));
         ArraySource((const byte *) enciv.c_str(), enciv.size(), true, new Base64Decoder(new ArraySink(iv, sizeof(iv))));
         ArraySource((const byte *) enccmac.c_str(), enccmac.size(), true, new Base64Decoder(new ArraySink(cmac_key, sizeof(cmac_key))));

         commonCmac.SetKey(cmac_key, cmac_key.size());
         commonE.SetKeyWithIV(common_key, common_key.size(), iv);
         commonD.SetKeyWithIV(common_key, common_key.size(), iv);
         symRead(commonD, commonCmac, &connectedSock, readBuff, sizeof(readBuff));
         otherName = readBuff;
         symWrite(commonE, commonCmac, &connectedSock, ownName.c_str(), ownName.length());
	 break;
      }
      ss << c;
      screenLock.unlock();
   }
   screenLock.unlock();
   lthread.join();
}

void incomingRequestHandler(CBC_Mode< AES >::Decryption dAES)
{
   Socket dummySock;
   byte * tempBuf;
   struct timeval timeout;
   timeout.tv_sec = 1;
   timeout.tv_usec = 0;
   dummySock.Create();
   incSock.Listen();
   while(!done && !incSock.ReceiveReady(&timeout))
   {
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;
   }
   screenLock.lock();
   if (done)
   {
      screenLock.unlock();
      return;
   }
   done = true;
   int len2read;
   incSock.Accept(dummySock, (sockaddr *) NULL, (socklen_t *) NULL);
   connectedSock = dummySock;
   //other stuff needed to connect
   connectedSock.Receive((byte *) &len2read, sizeof(int));
   tempBuf = new byte [len2read + 1];
   memset(tempBuf, 0, len2read + 1);
   int bytesRead = connectedSock.Receive(tempBuf, len2read);
   string unencTicket;
   cout << "Got encrypted ticket: " << tempBuf << endl;
   StringSource(tempBuf, bytesRead, true, new Base64Decoder(new StreamTransformationFilter(dAES, new StringSink(unencTicket))));
   cout << "Decrypted ticket" << endl;
   stringstream Ticket(unencTicket);
   string oName, encAesKey, enciv, enccmac;
   Ticket >> oName >> encAesKey >> enciv >> enccmac;


   SecByteBlock cmac_key(AES::DEFAULT_KEYLENGTH);
   SecByteBlock common_key(AES::DEFAULT_KEYLENGTH);
   byte iv[ AES::BLOCKSIZE ];
   ArraySource((const byte *) encAesKey.c_str(), encAesKey.size(), true, new Base64Decoder(new ArraySink(common_key, sizeof(common_key))));
   ArraySource((const byte *) enciv.c_str(), enciv.size(), true, new Base64Decoder(new ArraySink(iv, sizeof(iv))));
   ArraySource((const byte *) enccmac.c_str(), enccmac.size(), true, new Base64Decoder(new ArraySink(cmac_key, sizeof(cmac_key))));

   commonCmac.SetKey(cmac_key, cmac_key.size());
   commonE.SetKeyWithIV(common_key, common_key.size(), iv);
   commonD.SetKeyWithIV(common_key, common_key.size(), iv);

   otherName = (char *) tempBuf;
   symWrite(commonE, commonCmac, &connectedSock, ownName.c_str(), ownName.length());
   symRead(commonD, commonCmac, &connectedSock, (char *) tempBuf, sizeof(tempBuf));
   delete [] tempBuf;

   printf("\r%s\r", std::string(charsRead, ' ').c_str());
   printf("Other client has connected, press a key to continue...");
   screenLock.unlock();
}
