int StartsWith(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}
int ends_with(char *a,char *b)
{
    for(int i=0;i<strlen(a);i++)
    {
        if(a[i]==b[0])
        {
            for(int j=0;j<strlen(b);j++)
                if(a[i+j]!=b[j])
                    return 0;
            return 1;
        }
    }
    return 0;    
}
void get_root_servers(char rootdnsserver[100])
{
    char line[100];
    system("nslookup -type=ns . > roots.txt");
    FILE *fp = fopen("roots.txt", "r");
    while (fscanf(fp,"%s",line)!=EOF)
    {
        if(ends_with(line,".root-servers.net."))
        {
            strcpy(rootdnsserver,line);
            break;
        }
    }
}
void get_domain_server(char domainserver[100], char website[230],char rootdnsserver[100])
{
    char s2[169] = "nslookup -type=ns ",s3[100] = "", path[1035]="", *token;
    
    token = strtok(website, ".");
    int i = strncmp(token, "www", 3);
    if (i == 0)
        token = strtok(NULL, ".");
    char *lastpart = strtok(NULL, "\0");
    
    strcat(s2,lastpart);  
    strcat(s2," ");  
    strcat(s2, rootdnsserver);
    strcat(s2, " > ");
    
    strcat(s2, "domain.txt");

    system(s2);

    FILE *fp = fopen("domain.txt", "r");
    while (fgets(path, sizeof(path), fp) != NULL) {
        if(strstr(path, "nameserver"))
        {
            token=strtok(path, " = ");
            token=strtok(NULL, " = ");
            token[strcspn(token, "\n")] = 0;
            printf("----%s----\n", token);
            strcpy(domainserver, token);
            break;                    
        }
    }
}
void get_name_server(char domainserver[100], char website[100], char nameserver[100])
{
    char s2[169] = "nslookup -type=ns ",s3[100] = "", path[1035]="", *token, w[100];
    
    strcpy(w, website);
    token = strtok(website, ".");
    int i = strncmp(token, "www", 3);
    if (i == 0)
        token = strtok(NULL, ".");
    
    strcat(s2,w);  
    strcat(s2," ");  
    strcat(s2, domainserver);
    strcat(s2, " > ");
    
    strcat(s2, "nameserver.txt");
    system(s2);

    FILE *fp = fopen("nameserver.txt", "r");
    while (fgets(path, sizeof(path), fp) != NULL) {
        if(strstr(path, "nameserver"))
        {
            token=strtok(path, " = ");
            token=strtok(NULL, " = ");
            token[strcspn(token, "\n")] = 0;
            printf("----%s----\n", token);
            strcpy(nameserver, token);
            break;                    
        }
    }
}
int check(char *s,char c)
{
    int i,count=0;
     for(i=0;s[i];i++)  
    {
    	if(s[i]==c)
    	{
          count++;
		}
 	}
 	return count;	  
 }

void get_main_server(char result[100],char domainserver[100], char website[100], int type)
{
    char s2[169] = "nslookup -type=",s3[100] = "", path[1035]="", rootdnsserver[100]="", *token,w[100], nameserver[100];
    

    if(type == 1)
        strcat(s2, "aaaa ");
    else if(type == 2)
        strcat(s2, "a ");
    else if(type == 3)
        strcat(s2, "cname ");
    else
        strcat(s2, "ns ");
    strcpy(w, website);
    w[strlen(w)-1] = '\0';
    website[strlen(website)-1] = '\0';
    

    strcat(s2,w);  
    strcat(s2," ");  


    get_name_server(domainserver,w, nameserver);
    strcat(s2, nameserver);
    strcat(s2, " > ");
    strcat(s2, "cache/");
    strcat(s2, website);

    strcat(s3, "cache/"); 
    strcat(s3, website);
    
    if(type==1)
    {
        strcat(s2, "-aaaa");
        strcat(s3, "-aaaa");
    }
        
    if(type ==2)
    {
        strcat(s2, "-a");
        strcat(s3, "-a");
    }
    if (type == 3)
    {
        strcat(s2, "-cname");
        strcat(s3, "-cname");
    }
    if (type == 4)
    {
        strcat(s2, "-ns");
        strcat(s3, "-ns");
    }
    strcat(s2, ".txt");
    strcat(s3, ".txt");
    printf("%s - this is command\n",s2);
    system(s2);
    FILE *fp = fopen(s3,"r");

    if(type == 1 || type == 2)
    {
        int i = 0;
        while (fgets(path, sizeof(path), fp) != NULL) {
            if(strstr(path, "Address"))
            {
                if(i == 1)
                {
                    token=strtok(path, " = ");
                    token=strtok(NULL, " = ");
                    token[strcspn(token, "\n")] = 0;
                    printf("----%s----\n", token);
                    strcpy(result, token);
                    i++;
                    break;
                }
                else
                    i++;
                                    
            }
        }
        if(i == 1)
            strcat(result, "not found");
    }
    if(type == 4)
    {
        
        while (fgets(path, sizeof(path), fp) != NULL) {
            if(strstr(path, "nameserver"))
            {
                token=strtok(path, " = ");
                token=strtok(NULL, " = ");
                token[strcspn(token, "\n")] = 0;
                strcat(result, token);
                strcat(result, ";");         
            }
        }
    }
    if(type == 3)
    {
        int flag = 0;
        while (fgets(path, sizeof(path), fp) != NULL) {
            if(strstr(path, "canonical name"))
            {
            printf("%s\n", path);

                token=strtok(path, " = ");
                token=strtok(NULL, " = ");
                token=strtok(NULL, " = ");
                token[strcspn(token, "\n")] = 0;
                printf("%s - this is cname\n", token);
                strcat(result, token);   
                flag = 1;   
            }
        }
        if(flag == 0)
        {
            strcat(result, "not found");
        }
    }
    
}

char *nslookup_handle(char result[100], char args[100], int type)
{
    int i, n;
    char website[230]="", domainserver[100]="",rootdnsserver[100]="";
    char website_name[100]="";
    
    strcpy(website, args);
    strcpy(website_name, args);

    get_root_servers(rootdnsserver);
    printf("This is rootdns-%s\n", rootdnsserver);
    get_domain_server(domainserver, website,rootdnsserver);
    printf("This is domainserver-%s\n", domainserver);
    get_main_server(result, domainserver, website_name, type);
}