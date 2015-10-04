#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "common.h"

# define CHUNK_SIZE 255



int main(int argc, const char * argv[]) {

    unsigned short echoServPort;
    if(argc == 1){
        echoServPort = SERVER_PORT;
    }else if(argc == 2){
        echoServPort = atoi(argv[1]);
    }else{
        printf("wrong arguments number!");
        exit(1);
    }
    
    int servSock;
    if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("create socket failed!");
        exit(1);
    }
    
    struct sockaddr_in echoServAddr;
    memset(&echoServAddr, 0,sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(echoServPort);
    
    if(bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr))<0){
        printf("bind() failed");
        exit(1);
    }
    
    if(listen(servSock, MAXPENDING) < 0){
        printf("listen() failed");
        exit(1);
    }
    
    struct sockaddr_in echoClntAddr;
    int clntSock;
    int counter = 1;
    socklen_t clntlen = sizeof(echoClntAddr);;
    char *client_hello = malloc(CHUNK_SIZE);
    char *loginid = malloc(255);
    char *name = malloc(255);
    int cookie;
    char * server_status;

    for(;;){
        clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntlen);
        if(clntSock<0){
            printf("accpet failed!");
            exit(1);
        }

        //receive MESSAGE
        int size_recv = recv(clntSock, client_hello, CHUNK_SIZE,0 );
        char *pch;
        pch  = strtok(client_hello," ");
        while(pch != NULL){
            if(counter == 1){
                if(strcmp(pch, MAGIC_STRING)!=0){
                    printf("magic string are not the same!");
                }else{
                    server_status = pch;
                }
            }
            if(counter == 2){
                if(strcmp(pch, "HELLO") != 0){
                    printf("wrong HELLO message!");
                }
            }
            if(counter == 3){
                strcpy(loginid, pch);
            }
            if(counter == 4){
              int len = strlen(pch);
              pch[len-1] =  '\0';
              strcpy(name, pch);
          }
          if(counter >= 4){
              pch = strtok(NULL," ");
              break;
          }
          counter++;
          pch = strtok(NULL, " ");
      }

    char clntName[clntlen];
    if(inet_ntop(AF_INET, &echoClntAddr.sin_addr.s_addr, clntName,
       clntlen) != NULL) {
        printf("1ST %s HELLO %s %s %s:%d\n",MAGIC_STRING, loginid, name, clntName,echoClntAddr.sin_port);
    }
        //generate cookie
int total = 0;
int count = 0;
char *val = strtok(clntName, ".");
while(val != NULL && count < 4){
  total = total + atoi(val);
  count = count + 1;
  val = strtok(NULL, ".");
}
cookie = (total * 13) % 1111;
fflush(stdout);
char cookie_str[255];
sprintf(cookie_str, "%d", cookie);
fflush(stdout);
server_status = strcat(server_status, " STATUS ");
server_status = strcat(server_status, cookie_str);
server_status = strcat(server_status, " ");
server_status = strcat(server_status, inet_ntoa(echoClntAddr.sin_addr));
server_status = strcat(server_status ,":" );
char echoServPort_str[255] ;
sprintf(echoServPort_str, "%d",echoServPort );
server_status = strcat(server_status,echoServPort_str );
printf("2nd is %s\n", server_status);
fflush(stdout);
send(clntSock, server_status, strlen(server_status), 0);

//generate client bye
char * client_bye;
recv(clntSock, client_bye, CHUNK_SIZE,0 );
count = 1;
pch = strtok(client_bye," ");
while (pch != NULL){
    if (counter == 1 ){
        if(strcmp(pch, MAGIC_STRING) != 0){
            printf("Magic Strings are not the same");         
            exit(1); 
        }else{
            client_bye = pch; 
        }

    }
    if (counter == 2 &&strcmp(pch, "CLIENT_BYE") != 0 ){
        printf(" CLIENT_BYE does not match!");
        fflush(stdout);
    }    
    if (counter == 3 && atoi(pch) !=cookie){
        printf("Client sent back a baddddd COOOKIE");
        fflush(stdout);
    }

    if (counter >3){
        pch = strtok(NULL, " ");
        break;
    }
    count = count + 1;
    pch = strtok (NULL, " ");
}    






}
}
