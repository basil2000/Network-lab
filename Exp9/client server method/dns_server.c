#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>

FILE *l;
FILE *ipf;
char res[1024];
char ip[1024];
void check_A_NAME(char *ptr)
{

  char buf[1024];
  strcpy(buf, ptr);
  //printf("%d",strlen(ptr));
  if ((buf[0] == 'w') && (buf[1] == 'w') && (buf[2] == 'w') && (buf[3] == '.'))
  {
    return;
  }

  bzero(ptr, 1024);

  strcat(ptr, "www.");
  strcat(ptr, buf);
  return;
}

void get_ip(char *p)
{
  int flag = 0;
  int pos = 0;
  FILE *f2;
  f2 = fopen("ipfile", "r");
  char re;
  while (!feof(f2))
  {
    re = fgetc(f2);
    //printf("%c",re);
    if (re == ':')
    {
      flag++;
    }
    if (flag == 5)
    {
      if (!(re == ' ' || re == '\n' || re == ':'))
      {
        p[pos] = re;
        pos++;
      }
      if (re == '\n')
      {
        p[pos] = '\0';
        flag++;
      }
    }
  }
}

/*int check_local(char *ptr)
{
  l = fopen("local_cache", "r");
  char st[1024];
  int f = 0;
  int f1 = 0;

  while (!feof(l))
  {
    f++;
    fgets(st, "%s", l);

    char *s1;
    char *s2;

    s1 = strtok(st, " ");
    s2 = strtok(NULL, "\n");

    if (strcmp(s1, ptr) == 0)
    {
      strcat(res, s2);
      return 1;
    }
  }

  return 0;
}
*/

void parse_req(char *req, char *in)
{
  bzero(req, 1024);

  char *token;
  int pos = 0;
  token = strtok(in, " ");
  while (token != NULL)
  {
    //printf("%s\n", token);
    if (!((token[0] == '-' && token[1] == 'p') || (token[0] == '1')))
    {
      for (int i = 0; i < strlen(token); i++)
      {
        req[pos] = token[i];
        pos++;
      }
      req[pos] = ' ';
      pos++;
    }

    token = strtok(NULL, " ");
  }
  req[pos] = '\0';
  //printf("%s", req);
}

int main()
{
  int udpSocket, nBytes;
  char buffer[1024];
  struct sockaddr_in serverAddr, clientAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size, client_addr_size;
  int i;

  char in[1024];
  char req[1024];
  char res[1024];

  //Create UDP socket
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  //Configure settings in address struct
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(7891);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  //Bind socket with address struct
  bind(udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

  printf("Listening\n");
  int clientLen = sizeof(clientAddr);
  while (1)
  {
    nBytes = recvfrom(udpSocket, in, 1024, 0, (struct sockaddr *)&clientAddr, &clientLen);
    in[nBytes] = '\0';
    //printf("message from client : %s",buffer);
    //bzero(buffer,1024);

    //printf("enter the url\n");
    //scanf("%s", in);

    //check_A_NAME(in);

    /*if (check_local(in))
  {
    printf("%s", ip);
    return 0;
  }*/
    parse_req(req, in);

    //strcpy(req, "nslookup ");
    //strcat(req, in);
    strcat(req, "> ipfile");
    system(req);

    //printf("%s\n",req);

    //get_ip(res);
    //printf("%s\n",res);
    FILE *ff;
    ff = fopen("ipfile", "r");
    char ch;
    int j = 0;
    while (!feof(ff))
    {
      ch = fgetc(ff);
      res[j] = ch;
      j++;
    }
    sendto(udpSocket, res, j, 0, (struct sockaddr *)&clientAddr, clientLen);
  }
  return 0;
}
