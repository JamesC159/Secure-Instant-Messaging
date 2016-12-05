#include <clientapp.h>
#include <clienthelp.h>


std::string ownName;
std::string otherName;
bool done;
int charsRead;
std::stringstream ss;
std::mutex screenLock;

void startTalking(CBC_Mode< AES >::Encryption eAES, CBC_Mode< AES >::Decryption dAES, CMAC< AES > cmac, Socket *sock)
{
   symWrite(eAES, cmac, sock, ownName.c_str(), ownName.length());
   char buff [32];
   memset(buff, '\0', sizeof(buff));
   symRead(sock, buff, sizeof(buff));
   otherName = buff;
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
