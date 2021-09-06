#include <arpa/inet.h>	// htonl()
#include <netinet/in.h> // inet_ntoa()

#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define SIZE 1024

typedef struct client Client;

struct client
{
	int sockfd;
	struct sockaddr_in addr;
};

void getTime(char *t_str)
{
	time_t t;
	struct tm tm;

	t = time(NULL);
	tm = *gmtime(&t);
	strcpy(t_str, asctime(&tm));
}

void closeConn(Client client)
{
	printf("[ Disconnected to %s:%hu ]\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port));
	close(client.sockfd);
}

void *listentoClient(Client client)
{

	char recv_buffer[SIZE];
	char send_buffer[2 * SIZE];
	int recv_len;

	char *reqLine;
	char *method, *uri, *httpv;

	char responseBody[SIZE];
	char contLen[5];
	char time[20];

	while (1)
	{
		if ((recv_len = recv(client.sockfd, recv_buffer, sizeof(recv_buffer) - 1, 0)) == -1)
		{
			perror("recv() failed ");
		}
		else
		{
			if (recv_len == 0)
			{
				closeConn(client);
				break;
			}
			else
			{
				recv_buffer[recv_len] = '\0';

				reqLine = strtok(recv_buffer, "\n");
				printf("[ Request from %s:%hu ] %s\n", inet_ntoa(client.addr.sin_addr), ntohs(client.addr.sin_port), reqLine);

				method = strtok(reqLine, " ");

				uri = strtok(NULL, " ");
				httpv = strtok(NULL, "\n");

				FILE *fp;
				fp = fopen("mypage.html", "r");
				int f = 0;
				unsigned long int vb = 29;
				while (1)
				{
					char buf[1024];
					unsigned long int sf = fread(buf, sizeof(char), 1024, fp);
					if (sf == 0)
					{
						break;
					}
					if ((strcmp(method, "GET") == 0) && (strcmp(uri, "/mypage.html") == 0))
					{
						strcpy(send_buffer, "HTTP/1.1 200 OK\n");
						strcpy(responseBody, buf);
						//printf("%s\n",buf);
					}
					else if ((strcmp(method, "GET") == 0) && (strcmp(uri, "/") == 0))
					{
						f = 1;
						strcpy(send_buffer, "HTTP/1.1 200 OK\n");
						strcpy(responseBody, "Welcome to the Dept. of CSE!");
					}
					else
					{
						strcpy(send_buffer, "HTTP/1.1 404 Not Found\n");
						strcpy(responseBody, "<!DOCTYPE HTML><html><head><title>404 Not Found</title></head><body><b>Error 404</b><br></body></html>");
					}
					getTime(time);
					strcat(send_buffer, "Date: ");
					strcat(send_buffer, time);

					strcat(send_buffer, "Server: TCP HTTP Server\n");
					/*if(f==1)
					{ 
						strcat(send_buffer, "Content-Length: ");
						sprintf(contLen, "%lu",vb);
						strcat(send_buffer, contLen);
					}
					else
					{
						strcat(send_buffer, "Content-Length: ");
						sprintf(contLen, "%lu",sf);
						strcat(send_buffer, contLen);

					}
					*/
					strcat(send_buffer, "Content-Length: ");
					sprintf(contLen, "%lu", strlen(responseBody));
					strcat(send_buffer, contLen);
					
					strcat(send_buffer, "\nConnection: Keep-Alive\nKeep-Alive: timeout=5, max=100\n");
					strcat(send_buffer, "Content-Type: text/html\n\n");

					strcat(send_buffer, responseBody);

					if (send(client.sockfd, send_buffer, strlen(send_buffer), 0) < 0)
					{
						perror("send() fail");
					}
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{

	Client *clientList = NULL;

	//int socket(int domain, int type, int protocol)
	//int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)

	int sock;
	struct sockaddr_in server_addr;
	unsigned short port = 8080;

	//Server socket address parameters
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//Creating Socket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket() failed ");
		return 1;
	}

	//Setting socket options
	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
		perror("setsockopt() failed ");

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("bind() failed ");
		return 2;
	}
	else
		printf("Server Listening on port %hu\n", port);

	//int listen(int sockfd, int backlog)
	if (listen(sock, 5) == -1)
	{
		perror("listen() failed ");
		return 3;
	}

	Client newClient;
	struct sockaddr_in client_addr;
	int client_addrLen = sizeof(client_addr);
	int temp_sock;

	while (1)
	{
		if ((temp_sock = accept(sock, (struct sockaddr *)&client_addr, &client_addrLen)) < 0)
		{
			perror("accept() failed ");
			continue;
		}
		else
		{
			printf("[ Connected to %s:%hu ]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			if (fork() == 0)
			{
				close(sock);
				newClient.sockfd = temp_sock;
				newClient.addr = client_addr;
				listentoClient(newClient);
				break;
			}
			else
			{
				close(newClient.sockfd);
			}
		}
	}

	return 0;
}
