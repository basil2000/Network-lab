/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


//struct cache arr[20];

int main()
{
  int clientSocket, portNum, nBytes;
  char buffer[1024];
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(7891);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  addr_size = sizeof serverAddr;

  //init_cache();
  
  char in[1024];
  char res[1024];

  //nslookup -port=8080 nitc.ac.in 127.0.0.1
  //printf("enter the url\n");
  scanf("%[^\n]%*c", in);

  sendto(clientSocket, in, strlen(in), 0, (struct sockaddr *)&serverAddr, addr_size);

  nBytes = recvfrom(clientSocket, res, 1024, 0, NULL, NULL);
  res[nBytes]='\0';
  for(int i=0;i<nBytes;i++)
  {
    printf("%c",res[i]);
  }

  return 0;
}
