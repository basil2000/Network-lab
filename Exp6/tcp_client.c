

#include <sys/time.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

struct timeval start, stop;

int flag;
int n;
FILE *fp;
char *filename = "recv.mp4";
int count;
FILE *fpgr;
char *filename1 = "plot.txt";

void *graph(void *vargp)
{

  int c = 0;
  fpgr = fopen(filename1, "w");
  fprintf(fpgr, "%f\t%d\n", 0.0, 0);

  double sec = 0;

  int k = 1;
  gettimeofday(&start, NULL);

  while (1)
  {

    gettimeofday(&stop, NULL);
    sec = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
    if (sec >= 0.1)
    {

      int speed = (count - c) * 500 / 0.1;
      c = count;
      float xt = k * 0.1;
      k++;
      fprintf(fpgr, "%f\t%d\n", xt, speed);
      //memcpy(&stop, &start, sizeof(struct timeval));
      start.tv_sec=stop.tv_sec;
      start.tv_usec=stop.tv_usec;
      if (flag)
      {
        return NULL;
      }
    }
    
    
  }
}

int main()
{

  int flag = 0;

  int clientSocket;
  char buffer[1024];
  char msg[1024];
  socklen_t addr_size;
  struct sockaddr_in serverAddr;

  clientSocket = socket(PF_INET, SOCK_STREAM, 0);

  serverAddr.sin_family = AF_INET;

  serverAddr.sin_port = htons(5444);

  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  addr_size = sizeof serverAddr;

  connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size);

  int *addr = &clientSocket;

  bzero(buffer, 1024);
  bzero(msg, 1024);
  while (1)
  {

    scanf("%[^\n]%*c", buffer);
    strcpy(msg, buffer);

    if (strcmp(msg, "Bye") == 0)
    {
      close(clientSocket);
      return 0;
    }

    else if (strcmp(msg, "GivemeyourVideo") == 0)
    {

      send(clientSocket, msg, strlen(msg), 0);
      fp = fopen(filename, "w");

      pthread_t t;
      pthread_create(&t, NULL, graph, NULL);
      count = 0;

      while (1)
      {
        n = recv(clientSocket, buffer, 1024, 0);
        count++;
        //buffer[n] = '\0';
        //printf("%s\n",buffer);
        if (strcmp(buffer,"file_sent")==0)
        {
          flag = 1;
          printf("file sent successfully\n");
      
              while(1)
              {
                char buf[1024];
                scanf("%s",buf);
                if(strcmp(buf,"Bye")==0)
                {
                  close(clientSocket);
                  return 0;
                }
              }
              return 0;
            
          
          //
          //return 0;
        }
        buffer[n] = '\0';
        //printf("%s\n",buffer);
        //fprintf(fp, "%s", buffer);
        fwrite(buffer, sizeof(char), n, fp);
        bzero(buffer, 1024);
      }
    }
    else
    {
      send(clientSocket, msg, strlen(msg), 0);
      printf("type GivemeyourVideo to get video file\n");
    }
    bzero(buffer, 1024);
    bzero(msg, 1024);
  }
  return 0;
}
