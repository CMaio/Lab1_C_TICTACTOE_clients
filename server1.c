#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdbool.h>

#include <pthread.h>
#include <wait.h>

#define MAXPENDING  0   /* Maximum number of pending connections on server */
#define BUFFSIZE   255  /* Maximum number of bytes per message */

void err_sys(char *mess) { perror(mess); exit(1); }

char *board [5][5];
char *client = "X";
char *server = "O";
int turn = 1;
char boardfil[BUFFSIZE];

void VisualizeBoard(){

    printf("%s", "  1 2 3 4 5\n");
    for(int i = 0; i<5; i++){
        printf("%d", i+1);
        for(int j = 0; j<5; j++){
            printf("%s","|");
            printf("%s", board[i][j]);
        }
        printf("%s","|\n");
    }
}


void passboard(){
    
    strcpy(boardfil,"  1 2 3 4 5\n");
    for(int i = 0; i<5; i++){
        char pos[10] = {(i+1)+'0'};
        strcat(boardfil, pos);
        for(int j = 0; j<5; j++){
            strcat(boardfil,"|");
            strcat(boardfil,board[i][j]);
        }
        strcat(boardfil,"|\n");
    }
}


void SetUpBoard(){

    for(int i = 0; i<5; i++){
        for(int j = 0; j<5; j++){
            board[i][j] = " ";
        }
    }

    VisualizeBoard();
}

bool CheckValidPosition(int coordX, int coordY){

    if(board[coordX-1][coordY-1] != " "){return false;}
    return true;

}

void InsertMovement(int coordX, int coordY){

    switch(turn){
        case -1:
            board[coordX-1][coordY-1] = server;
        break;
        case 1:
            board[coordX-1][coordY-1] = client;
        break;
        default:
            printf("Error");
    }
    VisualizeBoard();
}

bool NotFinishedGame(){
    for(int i = 0; i<5; i++){
        for(int j = 0; j<5; j++){
            if(board[i][j] == " "){return true;}
        }
    }
    return false;
}

void closeProgram(int sock,int result){
    close(result);
    close(sock);
    exit(0);

}



void handle_client(int mysock) {
    //int *mysock = (int *)vargp; 
    char buffer[BUFFSIZE];
    int contador;
    int result;

    char* finishG;
    char* tie="It's a tie.\nGame finish\n";
    char* clientWin ="YOU WIN.\nGame finish\n";
    char* ServerWin="Server wins you lose.\nGame finish\n";
    char* lnplace="In which line would you like to place your movement? [1-5] ";
    char clplace[BUFFSIZE]={"In which column would you like to place your movement? [1-5] "};
    char outbound[BUFFSIZE]={"That's an out of bounds location, remember that the range of the matrix is [1-5] "};


    printf("Executing handleThread with socket descriptor ID: %d\n", mysock); 
    fprintf(stdout, "Thread ID in handler %lu\n", pthread_self());


    read(mysock,&buffer[0],BUFFSIZE);
    printf("message from client: %s\n", buffer);


    SetUpBoard();
    int coordX;
    char coordXLectura[10];
    int coordY;
    char coordYLectura[10];
    bool finishGame=false;
    char resultgame;
    char lineaLectura[10];
    finishG = tie;

    while(NotFinishedGame() && !finishGame){ 
        if(turn == -1){
            printf("\n");
            printf("In which line would you like to place your movement? [1-5] ");
            fgets(coordXLectura, 10, stdin);
            coordX = atoi(coordXLectura);

            while(coordX > 5 || coordX < 1){
                printf("That's an out of bounds location, remember that the range of the matrix is [1-5] ");
                fgets(coordXLectura, 10, stdin);
                coordX = atoi(coordXLectura);
            }

            printf("In which column would you like to place your movement? [1-5] ");
            fgets(coordYLectura, 10, stdin);
            coordY = atoi(coordYLectura);

            while(coordY > 5 || coordY < 1){
                printf("That's an out of bounds location, remember that the range of the matrix is [1-5] ");
                fgets(coordYLectura, 10, stdin);
                coordY = atoi(coordYLectura);
            }
            printf("\n");

            if(CheckValidPosition(coordX, coordY)){
                InsertMovement(coordX, coordY);
                turn *= -1;
                printf("Did the server win?[Y/N]"); 
                fgets(lineaLectura, 10, stdin);
                resultgame = lineaLectura[0];

                if(resultgame=='Y'){
                    printf("Server wins!!\n");
                    finishGame=true;
                    finishG=ServerWin;
                }   
            }else{
                printf("This position has already been taken. \n");
            }
        }else{
            passboard();
            strcat(boardfil,lnplace);

            write(mysock,boardfil,strlen(boardfil)+1); /*Wich position in line choose the player*/
            read(mysock,&buffer[0],BUFFSIZE); /*Recibe the position in line*/
            sscanf(buffer,"%d",&coordX); /*save the position as a int*/
            while(coordX > 5 || coordX < 1){
                write(mysock,outbound,strlen(outbound)+1); /*Wich position in line choose the player*/
                read(mysock,&buffer[0],BUFFSIZE); /*Recibe the position in line*/
                sscanf(buffer,"%d",&coordX); /*safe the position as a int*/
            }


            fprintf(stderr, "C: Position line selected: %s\n", buffer); /*print the line to be sure wich is the line*/
            write(mysock,clplace,strlen(clplace)+1); /*Wich position in column choose the player*/
            read(mysock,&buffer[0],BUFFSIZE); /*Recibe the position in column*/
            sscanf(buffer,"%d",&coordY); /*safe the position as a int*/
            while(coordY > 5 || coordY < 1){
                write(mysock,outbound,strlen(outbound)+1); /*Wich position in line choose the player*/
                read(mysock,&buffer[0],BUFFSIZE); /*Recibe the position in line*/
                sscanf(buffer,"%d",&coordY); /*safe the position as a int*/
            }
            fprintf(stderr, "C: Position column selected: %s\n", buffer);

            printf("Line: %d",coordX);
            printf("Column: %d",coordY);
            printf("\n");
            if(CheckValidPosition(coordX, coordY)){
                InsertMovement(coordX, coordY);
                turn *= -1;
                printf("Did the client win?[Y/N]");  
                fgets(lineaLectura, 10, stdin);
                resultgame = lineaLectura[0];

                if(resultgame=='Y'){
                    printf("Client win!!\n");
                    finishGame=true;
                    finishG=clientWin;
                }   
            }else{
                printf("This position has already been taken. \n");
            }
        }
    }
    passboard();
    printf("%s",boardfil);
    strcat(boardfil,finishG);
    printf("Game finish\n");
    write(mysock,boardfil,strlen(boardfil)+1);
    turn = 1;
    close(mysock);
}
int main(int argc, char *argv[]) {
    struct sockaddr_in echoserver, echoclient;
    int serversock, clientsock;
    int returnedpid, result;
    int pid, ppid;
    pthread_t handleThreadId;

    /* Check input parameters */
    if (argc != 2) {
    err_sys("Usage: server <port>\n");
    }

    /* Create TCP socket*/
    serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serversock < 0) {
    err_sys("Error socket");
    }

    /* Set sockaddr_in */
    memset(&echoserver, 0, sizeof(echoserver));       /* Reset memory */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Any address */
    echoserver.sin_port = htons(atoi(argv[1]));       /* Server port */

    /* Bind */
    if (bind(serversock, (struct sockaddr *) &echoserver,sizeof(echoserver)) < 0) {
	    err_sys("error bind");
    }

    /* Listen */
    if (listen(serversock, MAXPENDING) < 0) {
	    err_sys("error listen");
    }

    /* Loop */
    while (1) {
        unsigned int clientlen = sizeof(echoclient);
        fprintf(stdout, "PARENT PROCESS: Waiting for ACCEPT\n");
        clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen);
        if (clientsock< 0) {
            err_sys("error accept");
        }
        returnedpid = fork();

        /* Process child and parent processes */
        if (returnedpid < 0) {
            err_sys("Error fork");
        }
        else if (returnedpid > 0)
        {
            /* Parent process */
            wait(NULL);
            /* Close client socket */
            close(clientsock);
            
            err_sys("End of parent process");
        }
        else
        {
            /* Child process */

            /* Close server socket */
            close(serversock);

            fprintf(stdout, "Client: %s\n", inet_ntoa(echoclient.sin_addr));

            /* Handle client */
            handle_client(clientsock);
            exit(0);
            err_sys("End of child process");
        }    
    }  
}  