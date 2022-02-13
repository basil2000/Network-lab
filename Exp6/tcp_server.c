

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

struct clientData
{
  int sd;
  char Name[20];
};

int count;
struct clientData arr[10];

void *newClient(void *p1)
{

  char buff[1024];
  int nb;
  struct clientData *p = (struct clientData *)p1;

  bzero(buff, 1024);
  while (1)
  {
    if (p->sd != -20)
    {
      nb = recv(p->sd, buff, 1024, 0);
      buff[nb] = '\0';

      if (strcmp(buff, "GivemeyourVideo") == 0)
      {

        FILE *fp;
        char *filename = "send.mp4";
        fp = fopen(filename, "r");
        char data[1024];
        bzero(data, 1024);
        while (1)
        {
          int bs=fread(data, sizeof(char), 500, fp);
          if(bs==0)
          {
            bzero(data,1024);
            strcpy(data,"file_sent");
            send(p->sd,"file_sent",10,0);
            while(1)
            {
              int s=1;
            }
          }
          if (send(p->sd, data, bs, 0) == -1)
          {
            perror("[-]Error in sending file.");
            exit(1);
          }
          bzero(data, 1024);
        }
        //close(p->sd);
        //return NULL;
      }
    }
  }
}
int main()
{
  int welcomeSocket, newSocket;

  char buffer[1024];
  struct sockaddr_in serverAddr;
  struct sockaddr_in clientAddr;
  socklen_t addr_size;

  count = 0;

  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);

  serverAddr.sin_family = AF_INET;

  serverAddr.sin_port = htons(5444);

  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  bind(welcomeSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

  if (listen(welcomeSocket, 10) == 0)
    printf("server is live\n");
  else
    printf("Error\n");
  int i = 0;
  while (1)
  {

    char buff[1024];
    addr_size = sizeof clientAddr;

    newSocket = accept(welcomeSocket, (struct sockaddr *)&clientAddr, &addr_size);

    arr[count].sd = newSocket;
    count++;

    pthread_t t;

    pthread_create(&t, NULL, newClient, arr + i);
    i++;
  }
  return 0;
}
