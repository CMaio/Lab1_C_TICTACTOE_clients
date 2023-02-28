#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define BUFFSIZE 255

void err_sys(char *mess) { perror(mess); exit(0); }

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoserver;
    char buffer[BUFFSIZE];
    unsigned int echolen;
    int readnum,result;

    if (argc != 3) {
        fprintf(stderr, "USAGE: client <ip_server> <port>\n");
        exit(1);
    }
    /* we try to create TCP socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        err_sys("error socket");
    }

    /* we set information for sockaddr_in */
    memset(&echoserver, 0, sizeof(echoserver));       /* reset memory */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = inet_addr(argv[1]);  /* IP address */
    echoserver.sin_port = htons(atoi(argv[2]));       /* server port */

    /* we try to have a connection with the server */
    if (connect(sock,(struct sockaddr *) &echoserver,sizeof(echoserver)) < 0) {
        err_sys("error connect");
    }

    printf("Want to connect?\n");
    fgets(buffer, BUFFSIZE - 1, stdin);

    write(sock,buffer,strlen(buffer)+1);
    fprintf(stdout, " sent \n");

    while(1){
        if ((readnum = recv(sock, buffer, BUFFSIZE - 1, 0)) < 1) {
            err_sys("error reading\n");
        }
        if(strstr(buffer,"finish")!=NULL){
            fprintf(stdout, "%s\n",buffer);
            close(sock);
            exit(0);
        }
        fprintf(stdout, " %s \n",buffer);
        fgets(buffer, BUFFSIZE - 1, stdin);
        result = send(sock, buffer, strlen(buffer)+1, 0);
        if (result != strlen(buffer)+1) {
            err_sys("error writing");
        }
        fprintf(stdout, " sent \n");

    }
    close(sock);
    exit(0);

}