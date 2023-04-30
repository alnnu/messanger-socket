//Código para compilar e executar: g++ -pthread servidor.cpp -o servidor && ./servidor
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include <pthread.h>

/* Server port  */
#define PORT 4242

/* Buffer length */
#define BUFFER_LENGTH 4096

char buffer[BUFFER_LENGTH];


char *msg_cliente(char buffer[BUFFER_LENGTH]) {
    char *msg;
    char *aux = strtok(buffer, "|");
    int count = 1;

    while(strcmp(aux, "eom") != 0)
    {
        if(count == 2) {
            if(strcmp(aux, "usuario_entra") == 0) {
                count ++;
                aux=strtok(NULL, "|");
                msg = aux;
            }
        } else{
            count ++;
            aux=strtok(NULL, "|");
        }
    }
    return &(*msg);
}

char *gerar_string(char subString[BUFFER_LENGTH]) {

    char *aux = (char *)malloc(sizeof(char) * BUFFER_LENGTH);

    strcat(aux,"bom|msg_servidor|");
    strcat(aux, subString);
    strcat(aux, "|eom");

    printf("%s\n", aux);

    return &(*aux);
}

char *apelito_texto(char apelido[BUFFER_LENGTH]) {
    char *aux = (char *)malloc(sizeof(char) * BUFFER_LENGTH);


    strcat(aux, "Usuario ");
    strcat(aux, apelido);
    strcat(aux, " entrou");



    return &(*aux);
}

void *myThread(void *ptr ) {

    int client = *(int *)ptr;




    fprintf(stdout, "Client [%d] connected.\nWaiting for message ...\n",client);

        do {
            memset(buffer, 0x0, BUFFER_LENGTH);
            /* Receives client message */
            int message_len;
            if((message_len = recv(client, buffer, BUFFER_LENGTH, 0)) > 0) {
                buffer[message_len - 1] = '\0';
                printf("Client says[%d]: %s\n",client, buffer);
            }



            /* 'bye' message finishes the connection */
            if(strcmp(buffer, "bye") == 0) {
                send(client,"bom|msg_servidor|bye!|eom\0", BUFFER_LENGTH, 0);
            } else {
                send(client, "bom|msg_servidor|yep!|eom\0", BUFFER_LENGTH, 0);
            }

        } while(strcmp(buffer, "bye") != 0);


    close(client);
    free(ptr);

    /* Communicates with the client until bye message come */
    pthread_exit(NULL);
}

/*
 * Main execution of the server program of the simple protocol
 */
int main(void) {

    /* Client and Server socket structures */
    struct sockaddr_in client, server;

    /* File descriptors of client and server */
    int serverfd, clientfd;



    fprintf(stdout, "Starting server\n");

    /* Creates a IPv4 socket */
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd == -1) {
        perror("Can't create the server socket:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Server socket created with fd: %d\n", serverfd);


    /* Defines the server socket properties */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    memset(server.sin_zero, 0x0, 8);

    /* Handle the error of the port already in use */
    int yes = 1;
    if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,
                  &yes, sizeof(yes)) == -1) {
        perror("Socket options error:");
        return EXIT_FAILURE;
    }



    /* bind the socket to a port */
    if(bind(serverfd, (struct sockaddr*)&server, sizeof(server)) == -1 ) {
        perror("Socket bind error:");
        return EXIT_FAILURE;
    }

    /* Starts to wait connections from clients */
    if(listen(serverfd, 1) == -1) {
        perror("Listen error:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Listening on port %d\n", PORT);



//    /* Copies into buffer our welcome message */
//    strcpy(buffer, "Hello, client!\n\0");

    /* Sends the message to the client */
    socklen_t client_len = sizeof(client);
    while (1) {
        clientfd=accept(serverfd,(struct sockaddr *) &client, &client_len );

        if (clientfd == -1) {
            perror("Accept error:");
            return EXIT_FAILURE;
        }



        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = clientfd;
        pthread_t thread;

        //      inicio zona de conflito
        strncpy(buffer, "bom|msg_servidor|Olá! Seja bem-vindo!|eom\0", BUFFER_LENGTH);
        send(clientfd, buffer, BUFFER_LENGTH, 0);

        recv(clientfd, buffer, BUFFER_LENGTH, 0);

        //refazer tem que escrever no buffer, criar uma thread que manda para toda. Somente le;

        char *apelido = apelito_texto(msg_cliente(buffer));
        printf("---%s\n", apelido);
        char *send_texto = gerar_string(apelido);
        send(clientfd,send_texto , BUFFER_LENGTH, 0);

        //      final zona de conflito
        pthread_create(&thread, NULL, myThread, socket_ptr);
    }


    /* Close the local socket */
    close(serverfd);

    printf("Connection closed\n\n");


    return EXIT_SUCCESS;
}

