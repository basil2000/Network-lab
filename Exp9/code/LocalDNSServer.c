#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>

#include "DNS.c"
#define SIZE 2048
#define QSIZE 100

typedef struct DNSPacketHeader DNSHeader;
typedef struct DNSPacketQuestion DNSQuestion;
typedef struct DNSPacketAnsAddition DNSAns;
typedef struct DNSRecord DNSRecord;

typedef struct handleLookupArg HLArg;

struct DNSPacketHeader
{
    char id[2];
    short unsigned QR;
    short unsigned Opcode;
    short unsigned AA;
    short unsigned TC;
    short unsigned RD;
    short unsigned RA;
    short unsigned Z;
    short unsigned AD;
    short unsigned CD;
    short unsigned RCODE;
    short unsigned QDCOUNT;
    short unsigned ANCOUNT; // number of answer entries
    short unsigned NSCOUNT;
    short unsigned ARCOUNT;
};

struct DNSPacketQuestion
{
    char QNAME[QSIZE];
    short unsigned qsize;
    char QTYPE[2];
    char QCLASS[2];
};

struct DNSPacketAnsAddition
{
    unsigned short TTL;
    unsigned short RDLENGTH;
    char RDATA[QSIZE];
};

struct DNSRecord
{
    DNSQuestion Q;
    DNSAns A;

    struct DNSRecord *next;
    struct DNSRecord *prev;
};

struct handleLookupArg
{
    int sock;
    char Buf[SIZE];
    struct sockaddr_in clientAddr;
};

DNSRecord *Cache;
pthread_mutex_t cache_lock;

void getTime(char *t_str)
{
    time_t t;
    struct tm tm;

    t = time(NULL);
    tm = *localtime(&t);
    strcpy(t_str, asctime(&tm));
}

void parseHeader(char *recvBuf, DNSHeader *req)
{

    char octect;

    req->id[0] = recvBuf[0];
    req->id[1] = recvBuf[1];

    octect = recvBuf[2];
    req->QR = (octect & 128) >> 7;
    req->Opcode = (octect & 120) >> 3;
    req->AA = (octect & 4) >> 2;
    req->TC = (octect & 2) >> 1;
    req->RD = octect & 1;

    octect = recvBuf[3];
    req->RA = (octect & 128) >> 7;
    req->Z = (octect & 64) >> 6;
    req->AD = (octect & 32) >> 5;
    req->CD = (octect & 16) >> 4;
    req->RCODE = octect & 15;

    req->QDCOUNT = recvBuf[4];
    req->QDCOUNT = (req->QDCOUNT) << 8;
    req->QDCOUNT = (req->QDCOUNT) + recvBuf[5];

    req->ANCOUNT = recvBuf[6];
    req->ANCOUNT = (req->ANCOUNT) << 8;
    req->ANCOUNT = (req->ANCOUNT) + recvBuf[7];

    req->NSCOUNT = recvBuf[8];
    req->NSCOUNT = (req->NSCOUNT) << 8;
    req->NSCOUNT = (req->NSCOUNT) + recvBuf[9];

    req->ARCOUNT = recvBuf[10];
    req->ARCOUNT = (req->ARCOUNT) << 8;
    req->ARCOUNT = (req->ARCOUNT) + recvBuf[11];
}

void fetchQuestion(char *qstart, DNSQuestion *qstn)
{
    int i = 0;
    while (qstart[i])
    {
        i = i + (qstart[i] + 1);
    }

    qstn->qsize = i + 1;

    for (i = 0; i < qstn->qsize; ++i)
        qstn->QNAME[i] = qstart[i];
}
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
void parseQuestion(char *qStart, DNSQuestion *qstn)
{
    fetchQuestion(qStart, qstn);

    qstn->QTYPE[0] = qStart[qstn->qsize];
    qstn->QTYPE[1] = qStart[qstn->qsize + 1];

    qstn->QCLASS[0] = qStart[qstn->qsize + 2];
    qstn->QCLASS[1] = qStart[qstn->qsize + 3];
}

void parseIPv4(char *RDATA, char *ip)
{
    char *octect = strtok(ip, ".");
    RDATA[0] = atoi(octect);
    octect = strtok(NULL, ".");
    RDATA[1] = atoi(octect);
    octect = strtok(NULL, ".");
    RDATA[2] = atoi(octect);
    octect = strtok(NULL, "\0");
    RDATA[3] = atoi(octect);
}
void check_4(char final[4], char *octet)
{
    strcpy(final, "");
    if(strlen(octet) == 1)
    {
        strcat(final,"000");
        strcat(final, octet);
    }
    else if(strlen(octet) == 2)
    {
        strcat(final,"00");
        strcat(final, octet);
    }
    else if(strlen(octet) == 3)
    {
        strcat(final,"0");
        strcat(final, octet);
    }
    else
    strcat(final, octet);
}
char* substr(const char *src, int m, int n)
{
    int len = n - m;
 
    char *dest = (char*)malloc(sizeof(char) * (len + 1));
    for (int i = m; i < n && (*(src + i) != '\0'); i++)
    {
        *dest = *(src + i);
        dest++;
    }
    *dest = '\0';
    return dest - len;
}
void string2hexString(char* input, char* output)
{
    int loop;
    int i; 
    i=0;
    loop=0;
    while(input[loop] != '\0')
    {
        sprintf((char*)(output+i),"%02X", input[loop]);
        loop+=1;
        i+=2;
    }
    //insert NULL at the end of the output string
    output[i++] = '\0';
}

int cname( char *RDATA,char re[100], char website[100])
{
    char r[100],*token;
    strcpy(r, re);
    int i = 0,loop = 0;
    token = strtok(r, ".");
    while(token!=NULL)
    {
        //printf("%s %d\n", token, strlen(token));
        short m = strlen(token);
        RDATA[i] = m;
        i++;
        loop = 0;
        while(token[loop] != '\0')
        {
            RDATA[i] = token[loop];
            loop++;
            i++;
        }
        token=strtok(NULL, ".");
    }
   
    RDATA[i++] = 0;
    return i;
}
void parseIPv6(char *RDATA, char *ip2)
{
    char *ip,*octect;
    char final[4], format[20];
    
    if(strstr(ip2,"::"))
    {
        ip = str_replace(ip2,"::",":0000:0000:");
        octect = strtok(ip, ":");
        //sprintf(format, "%%0%dd", 4);
        //sprintf(final, format, octect);
        check_4(final, octect);

    }
    else
    {
        octect = strtok(ip2, ":");
        check_4(final, octect);

        
    }
    char byte[2];
    char *ptr;
    RDATA[0] = strtoumax(substr(final,0,2), &ptr, 16); //atoi(octect);
    RDATA[1] = strtoumax(substr(final,2,4), &ptr, 16); //atoi(octect);

    short i = 1;
    for (short i = 1; i < 6; ++i)
    {
        octect = strtok(NULL, ":");
        check_4(final, octect);
        RDATA[i*2] = strtoumax(substr(final,0,2), &ptr, 16); //atoi(octect);
        RDATA[i*2+1] = strtoumax(substr(final,2,4), &ptr, 16); //atoi(octect);

     //   printf("%s %x %x %i-\n", final,RDATA[i*2], RDATA[i*2+1],i);
        
    }

    octect = strtok(NULL, "\0");
    check_4(final, octect);
    
    printf("%s\n", octect);
    RDATA[14] = strtoumax(substr(final,0,2), &ptr, 16); //atoi(octect);
    RDATA[15] = strtoumax(substr(final,2,4), &ptr, 16); //atoi(octect);

}

int fetchFromCache(DNSQuestion *qstn, DNSAns *ans) //[TODO]
{
    printf("\n[ Fetching from Cache ]\n");
    DNSRecord *entry = Cache;
    int found = 0;

    while (entry != NULL)
    {
        if (qstn->qsize == entry->Q.qsize)
        {
            if (qstn->QTYPE[0] == entry->Q.QTYPE[0] && qstn->QTYPE[1] == entry->Q.QTYPE[1])
            {
                short matching = 1;
                for (unsigned short i = 0; i < qstn->qsize; ++i)
                {
                    if (qstn->QNAME[i] != entry->Q.QNAME[i])
                    {
                        matching = 0;
                        break;
                    }
                }
                if (matching == 1)
                {
                    *ans = entry->A;
                    found = 1;
                    break;
                }
            }
        }

        entry = entry->next;
    }
    if (found == 0)
        printf("\n[ Not found in Cache ]\n");
    return found;
}

void DNSNameToString(char *str, DNSQuestion *qstn)
{
    unsigned short i = 0, j = 0;
    while (qstn->QNAME[i])
    {
        j = i + 1;
        i = i + (qstn->QNAME[i] + 1);
        while (j < i)
        {
            str[j - 1] = qstn->QNAME[j];
            ++j;
        }
        str[j - 1] = '.';
    }
    str[j] = '\0';
}

void addToCache(DNSQuestion *qstn, DNSAns *ans)
{
    DNSRecord *entry = (DNSRecord *)malloc(sizeof(DNSRecord));

    entry->Q = *qstn;
    entry->A = *ans;
    entry->next = NULL;
    entry->prev = NULL;

    if (Cache != NULL)
    {
        entry->next = Cache;
        Cache->prev = entry;
    }
    Cache = entry;
    printf("\n[Added to Cache: %s]\n", qstn->QNAME);
}



int resolveQuery2(DNSQuestion *qstn, DNSAns *ans)
{
    /*
    type 1 = aaaa
    type 2 = a
    type 3 = cname
    type 4 = ns
    */

    ans->TTL = 30;
    int n = 1;
    char web[100],website[100],result[100]="",filename[100]="",*token, w[100],path[100];
    DNSNameToString(web, qstn);
    strcpy(w, web);
    strcat(filename, "cache/");
    strcat(filename, web);
    if( qstn->QTYPE[1] == 0x1C)
    {
        nslookup_handle(result, web,1);
        printf("this is result - %s\n", result);
        if(strncmp(result, "not found", 9) == 0)
        {
            ans->RDLENGTH = 0;
        }
        else
        {   printf("this is result - %s\n", result);
            ans->RDLENGTH = 16;
            parseIPv6(ans->RDATA, result);
        }
        
        addToCache(qstn, ans);
    }
    else if( qstn->QTYPE[1] == 0x1)
    {
        nslookup_handle(result, web,2);
        printf("this is result - %s\n", result);
        if(strncmp(result, "not found", 9) == 0)
        {
            ans->RDLENGTH = 0;
        }
        else
        {   printf("this is result - %s\n", result);
            ans->RDLENGTH = 4;
            parseIPv4(ans->RDATA, result);
        }
        addToCache(qstn, ans);
    }
    else if( qstn->QTYPE[1] == 0x5 || qstn->QTYPE[1] == 0x2)
    {
        char w[100];
        strcpy(w, web);
        strcpy(website, web);
        token = strtok(w, ".");
        int i = strncmp(token, "www", 3);
        
        if (i == 0)
        {   
            token = strtok(NULL, "\0");
            strcpy(website, token);
        }
                  
        printf("%s %d -this is result\n", website,i);

        nslookup_handle(result, web,3);
        if(strncmp(result, "not found", 9) == 0)
        {
            ans->RDLENGTH = 0;
        }
        else
        {
            int r = cname(ans->RDATA, result, website);
            ans -> RDLENGTH = r;
            printf("%s %i-this is result\n", website,r);
            return 666;
        }
    }
    else
    {
        
        nslookup_handle(result, web,4);
    }
    return n;
}
int resolveQuery(DNSQuestion *qstn, DNSAns *ans)
{
    ans->TTL = 30;
    int n = 1;
    if (fetchFromCache(qstn, ans) == 0)
    {
        n = resolveQuery2(qstn, ans);
    }
    return n;
}
void assignHeader(char *sendBuf, DNSHeader *head, unsigned short RDLENGTH)
{
    sendBuf[0] = head->id[0];
    sendBuf[1] = head->id[1];

    char byte = 1; //QR = 1 for response

    byte = byte << 4;
    byte = byte | (head->Opcode);

    byte = byte << 1;
    byte = byte | (head->AA); //[Modify AA]

    byte = byte << 1;
    byte = byte | (head->TC);

    byte = byte << 1;
    byte = byte | (head->RD);

    sendBuf[2] = byte;

    byte = 0; //RA

    byte = byte << 7;
    byte = byte | (head->RCODE);

    sendBuf[3] = byte;

    sendBuf[5] = head->QDCOUNT;
    sendBuf[7] = (RDLENGTH == 0) ? 0 : 1; //[Modify ANCOUNT]
    sendBuf[9] = head->NSCOUNT;
    sendBuf[11] = head->ARCOUNT;
}

void assignQuestion(char *qstField, DNSQuestion *qstn)
{
    unsigned i = 0;
    while (i < qstn->qsize)
    {
        qstField[i] = qstn->QNAME[i];
        ++i;
    }
    qstField[i++] = qstn->QTYPE[0];
    qstField[i++] = qstn->QTYPE[1];

    qstField[i++] = qstn->QCLASS[0];
    qstField[i++] = qstn->QCLASS[1];
}

void assignAnswer(char *ansField, DNSQuestion *qstn, DNSAns *ans)
{
    unsigned i = 0;
    while (i < qstn->qsize)
    {
        ansField[i] = qstn->QNAME[i];
        ++i;
    }
    ansField[i++] = qstn->QTYPE[0];
    ansField[i++] = qstn->QTYPE[1];

    ansField[i++] = qstn->QCLASS[0];
    ansField[i++] = qstn->QCLASS[1];

    ansField[i++] = 0;
    ansField[i++] = 0;
    ansField[i++] = 0;
    ansField[i++] = ans->TTL; //[Modify TTL]

    ansField[i++] = 0; //[Modify RDLENGTH]
    ansField[i++] = ans->RDLENGTH;

    for (unsigned j = 0; j < ans->RDLENGTH; ++j)
    {
        ansField[i++] = ans->RDATA[j];
    }
}

unsigned createResponse(DNSHeader *head, DNSQuestion *qstn, DNSAns *ans, char *sendBuf, int n)
{
    memset(sendBuf, 0, SIZE);

    unsigned pos = 0;

    assignHeader(sendBuf, head, ans->RDLENGTH);
    pos = 12;

    assignQuestion(sendBuf + pos, qstn);
    pos += (qstn->qsize) + 4;

    if (ans->RDLENGTH != 0)
    {
        assignAnswer(sendBuf + pos, qstn, ans);
        pos += (qstn->qsize) + 10 + (ans->RDLENGTH);
    }

    return pos;
}

void *TTLHandler()
{
    pthread_mutex_lock(&cache_lock);
    DNSRecord *entry, *tmp;
    entry = Cache;
    while (entry != NULL)
    {
        entry->A.TTL -= 1;

        if (entry->A.TTL == 0)
        {
            if (entry->next != NULL)
                (entry->next)->prev = entry->prev;

            if (entry->prev == NULL)
                Cache = entry->next;
            else
                (entry->prev)->next = entry->next;
            tmp = entry;
            printf("\n[ Deleting from Cache : %s ]\n", tmp->Q.QNAME);
            entry = entry->next;
            free(tmp);
        }
        else
            entry = entry->next;
    }
    pthread_mutex_unlock(&cache_lock);
}

void *cacheHandler()
{
    clock_t start, end;
    while (1)
    {
        start = clock() / CLOCKS_PER_SEC;
        while (1)
        {
            end = clock() / CLOCKS_PER_SEC;
            if (end - start >= 1)
                break;
        }
        pthread_t TId;
        pthread_create(&TId, NULL, TTLHandler, NULL);
    }
}

void *handleLookup(void *Arg)
{
    HLArg *arg = (HLArg *)Arg;

    DNSHeader reqHeader;
    DNSQuestion reqQstn;
    DNSAns ans;

    parseHeader(arg->Buf, &reqHeader);
    parseQuestion(arg->Buf + 12, &reqQstn);
    int n;
    if(reqQstn.QTYPE[1] == 0x2)
    {

       n = resolveQuery(&reqQstn, &ans);
       DNSAns ans[n];
    }
    else
    {
        int n = resolveQuery(&reqQstn, &ans);
    }

    char sendBuf[SIZE];
    unsigned packetSize;
    packetSize = createResponse(&reqHeader, &reqQstn, &ans, sendBuf,n);

    if (sendto(arg->sock, sendBuf, packetSize, 0, (struct sockaddr *)&(arg->clientAddr), sizeof(arg->clientAddr)) < 0)
        perror("sendto() failed");
    else
        printf("\n---------------------[ Response Sent ]---------------------\n");
    free(arg);
}

int main(int argc, char *argv[])
{
    Cache = NULL;
    pthread_mutex_init(&cache_lock, NULL);
    unsigned short my_port;
    if (argc < 2)
    {
        printf("\nPORT : ");
        scanf("%hu", &my_port);
    }
    else
        my_port = atoi(argv[1]);
    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket() failed");
        return 1;
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
        perror("setsockopt() failed");
    //bind socket to address
    struct sockaddr_in server_addr;

    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(my_port);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() failed");
        return 1;
    }

    printf("Server Listening on port %hu\n", my_port);

    pthread_t cT;
    pthread_create(&cT, NULL, cacheHandler, NULL);

    struct sockaddr_in clientAddr;
    int client_addrLen = sizeof(clientAddr);
    int temp_sock;

    char recvBuf[SIZE];
    int recvLen;

    HLArg *arg;
    while (1)
    {
        if ((recvLen = recvfrom(sock, recvBuf, SIZE - 1, 0, (struct sockaddr *)&clientAddr, &client_addrLen)) < 0)
        {
            perror("recvfrom() failed");
        }
        else
        {
            arg = (HLArg *)malloc(sizeof(HLArg));
            for (int i = 0; i < SIZE; ++i)
            {
                arg->Buf[i] = recvBuf[i];
            }
            arg->sock = sock;
            arg->clientAddr = clientAddr;
            pthread_t tId;
            pthread_create(&tId, NULL, handleLookup, (void *)arg);
            printf("\n---------------[ RequestHandler assigned ]-----------------\n");
        }
    }

    return 0;
}