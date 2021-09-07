// Shuvam Raj Satyal, 13102529
// Cedric Lim, 24026891

#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include<errno.h> //For errno - the error number

#define _BSD_SOURCE

struct hostent *gethostbyname(const char *name);

#define PORT 30000
#define MAXLINE 30
#define MAXARGC 30

void parseline(char* cmdline, const char *argv[]) {
	const char delimiter[2] = " ";
	char *token;
	int counter = 0;

	// Remove trailing newline character from input
	char *temp_pos;
	if ((temp_pos = strchr(cmdline, '\n')) != NULL) *temp_pos = '\0';

	// strtok returns one token at a time
	token = strtok(cmdline, delimiter);
	while (token != NULL) {
		argv[counter++] = token;
		token = strtok(NULL, delimiter);
	}
}

int hostname_to_ip(const char * hostname , char* ip)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
		
	if ( (he = gethostbyname( hostname ) ) == NULL) 
	{
		// get the host info
		herror("gethostbyname");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;
	
	for(i = 0; addr_list[i] != NULL; i++) 
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	
	return 1;
}

int main(int argc, char const *argv[]) // ./client server.ics.uci.edu 300000
{ 
    // printf("argv[1]:%s\n", argv[1]); //server.ics.uci.edu, NEED TO MODIFY CODE to account for this domain. 
    // printf("argv[2]:%s\n", argv[2]); //300000
    // struct hostent *lh = gethostbyname(argv[1]);
    // printf("host: %d\n", lh->h_addr);

    // START OF SOCKET
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(atoi(argv[2])); // PORT 

    const char *hostname = argv[1];
	char ip[100];
	
	hostname_to_ip(hostname , ip);
	// printf("%s resolved to %s" , hostname , ip);
	
	// printf("\n");

    // Convert IPv4 and IPv6 addresses from text to binary form ?
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)  // 2nd param use to be hard-coded as "127.0.0.1"
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    
    char cmd[MAXLINE]; // content to send to server 
    
    char cmdline[MAXLINE];

    while (1) {
        char buffer[1024] = {0}; 
		// Display prompt and get user input
		printf("> ");
		fgets(cmdline, MAXLINE, stdin);
		if (feof(stdin)) exit(0);  

		parseline(cmdline, argv);
		
        // printf("user input1: %s\n", argv[0]);
        // printf("user input2: %s\n", argv[1]);
		// printf("user input3: %s\n", argv[2]);

        int buf_size = 100;
        char buf [buf_size];
        int cx;

        // cx = snprintf(buf, buf_size, "%s %s %s", argv[0], argv[1], argv[2]);
        // if (cx >= 0 && cx < 100) {
        //     snprintf(butter + cx, buf_size - cx, "");
        // }
        
        if (strcmp(argv[0], "Prices") == 0 ) {
            // Prices function
            // argv[1] is stock_name
            // argv[2] is date
            cx = snprintf(buf, buf_size, "%s %s %s", argv[0], argv[1], argv[2]); 
            strcpy(cmd, buf);
        }

        else if (strcmp(argv[0], "MaxProfit") == 0) {
            // MaxProfit function
            // argv[1] stock_name
            cx = snprintf(buf, buf_size, "%s %s", argv[0], argv[1]);
            strcpy(cmd, buf);
        }

        else if (strcmp(argv[0], "quit") == 0) break;

        else {
            printf("Invalid Command\n");
            continue;
        }

        // printf("buf: %s\n", buf); // need to parse buf
        // printf("cmd: %s\n", cmd);
        

        send(sock , cmd , strlen(cmd) , 0 ); // send content "cmd" to server
        // printf("Sent cmd to server\n");

        valread = read( sock , buffer, 1024); // recieve content from server in the "buffer" variable
        printf("%s\n",buffer );  // print out content sent from server
	}
    return 0; 
} 