#include <clientapp.h>
#include <clienthelp.h>


std::string ownName;
std::string otherName;
bool done;
int charsRead;
std::stringstream ss;
std::mutex screenLock;


void incomingRequestHandler()
{
   incSock.Listen();

}

void startTalking(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket *sock)
{
   done = false;
   charsRead = 0;
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
   char readBuff [1024];
   done = false;
   charsRead = strlen("Who would you like to talk to?: ");
   ss.str("Who would you like to talk to?: ");
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
      if (c == '\n')
      {
         symWrite(eAES, cmac, sock, ss.str().c_str(), ss.str().length());
         memset(readBuff, 0, sizeof(readBuff));
         symRead(dAES, cmac, sock, readBuff, sizeof(readBuff));
         // Any other work with server and other client to connect
         done = true;
         charsRead = 0;
	 ss.str("");
	 break;
      }
      screenLock.unlock();
   }
   screenLock.unlock();
}
