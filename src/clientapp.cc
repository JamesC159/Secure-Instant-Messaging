#include <clientapp.h>


char ownName [32];
char otherName [32];
bool done;
int charsRead;
std::stringstream ss;
std::mutex screenLock;

void startTalking(int sock)
{
   done = false;
   charsRead = 0;
   ss.str("");
   std::thread lthread(sockListener, sock);
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
	 write(sock, ss.str().c_str(), charsRead);
	 printf("\r%s\r", std::string(charsRead, ' ').c_str());
	 printf("%s: %s\n", ownName, ss.str().c_str());
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

void sockListener(int sock)
{
   char buff [1024];
   memset(buff, '\0', sizeof(buff));
   while (true)
   {
      int cRead = read(sock, buff, sizeof(buff)-1);
      if (cRead > 0)
      {
         buff[cRead] = '\0';
	 screenLock.lock();
	 printf("\r%s\r", std::string(charsRead, ' ').c_str());
	 printf("%s: %s\n", otherName, buff);
	 if (std::string(buff) == "EXIT")
	 {
	    if (!done)
	    {
	       write(sock, buff, strlen(buff));
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
