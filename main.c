//CÃ³digo para compilar e executar: g++ -pthread servidor.cpp -o servidor && ./servidor
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
#include <semaphore.h>

#include "clients.h"


sem_t semaforoProdutor, semaforoConsumidor, mudex;

Lista* clients;


/* Server port  */
#define PORT 4242

/* Buffer length */
#define BUFFER_LENGTH 4096

char buffer[BUFFER_LENGTH];


struct infos{
    char *tipo;
    char *info;
    char *msg;
};
/*
 * Thread para os clientes(produtora)
 */



struct infos msg_client_decoder(char *msg, char *apelido) {

    struct infos dados;
    char *aux = strtok(msg, "|");
    int count = 1;

    while(strcmp(aux, "eom") != 0)
    {
        if(count == 2) {
            if(strcmp(aux, "usuario_entra") == 0 ) {
                dados.tipo = aux;
                count ++;
                aux=strtok(NULL, "|");
                dados.info = aux;

                dados.msg = (char*) malloc((sizeof(char) * BUFFER_LENGTH));
                strcat(dados.msg, "usuario ");
                strcat(dados.msg, aux);
                strcat(dados.msg, " entrou");
            } else if(strcmp(aux, "usuario_sai") == 0){
                dados.tipo = aux;
                count ++;
                aux=strtok(NULL, "|");
                dados.info = aux;

                dados.msg = (char*) malloc((sizeof(char) * BUFFER_LENGTH));
                strcat(dados.msg, "usuario ");
                strcat(dados.msg, aux);
                strcat(dados.msg, "saiu");
            } else if(strcmp(aux, "msg_cliente") == 0) {
                dados.tipo = aux;
                count ++;
                aux=strtok(NULL, "|");
                dados.info = aux;

                dados.msg = (char*) malloc((sizeof(char) * BUFFER_LENGTH));
                strcat(dados.msg, apelido);
                strcat(dados.msg, " falou: ");
                strcat(dados.msg, aux);
            }
        } else{
            count ++;
            aux=strtok(NULL, "|");
        }
    }
    return dados;
}


char *make_msg(char *type, char *text) {
    char *msg = (char*) malloc(sizeof(char) * BUFFER_LENGTH);

    strcat(msg,"bom|");
    strcat(msg,type);
    strcat(msg, "|");
    strcat(msg, text);
    strcat(msg, "|eom");

    return msg;
}


void *myThread(void *ptr ) {

    int client = *(int *)ptr;
    char *client_apelido;
    struct infos msg_dados;

    char *msg = (char*)malloc(sizeof (char) * BUFFER_LENGTH);

    int message_len;


    sem_wait(&mudex);
    strncpy(buffer, "Olá! Seja bem-vindo!\0", BUFFER_LENGTH);

    send(client, make_msg("msg_servidor",buffer), BUFFER_LENGTH, 0);

    sem_post(&mudex);

    sem_post(&semaforoProdutor);

    message_len = recv(client, msg, BUFFER_LENGTH, 0);


    sem_wait(&mudex);
    msg_dados = msg_client_decoder(msg,NULL);
    strncpy(buffer, msg_dados.msg, BUFFER_LENGTH);

    add(client, msg_dados.info);
    client_apelido = (char *) malloc(sizeof (char) * strlen(msg_dados.info));
    strcpy(client_apelido, msg_dados.info);

    sem_post(&mudex);

    while (1) {
        while(strlen(buffer) == BUFFER_LENGTH){
            fprintf(stdout,"Produtor -- o buffer está cheio! A produção está em espera!\n");
            sem_wait(&semaforoConsumidor);
            fprintf(stdout,"Produtor -- a produção voltou a funcionar!\n");
        }
        /* Receives client message */

        if((message_len = recv(client, msg, BUFFER_LENGTH, 0)) > 0) {

            sem_wait(&mudex);
            msg_dados = msg_client_decoder(msg, client_apelido);

            strcpy(buffer, msg_dados.msg);

            buffer[message_len - 1] = '\0';

            fprintf(stdout,"from client: %s", buffer);

            /* 'bye' message finishes the connection */
            if(strcmp(msg_dados.tipo, "usuario_sai") == 0) {
                send(client,make_msg("msg_servidor", "bye!"), BUFFER_LENGTH, 0);
                sem_post(&mudex);

                sem_post(&semaforoProdutor);
                break;
            } else {
                send(client, make_msg("msg_servidor",buffer), BUFFER_LENGTH, 0);

            }

            sem_post(&mudex);
            sem_post(&semaforoProdutor);

        }
    }


    close(client);
    free(ptr);
    free(msg);
    remover(client);

    /* Communicates with the client until bye message come */
    pthread_exit(NULL);
}


/*
 * Main execution of the server program of the simple protocol
 */

int main(void) {
    clients = create_clients_list();

    sem_init(&semaforoProdutor, 0, 0);
    sem_init(&semaforoConsumidor, 0, 0);
    sem_init(&mudex, 0, 1);

    /* Client and Server socket structures */
    struct sockaddr_in client, server;

    /* File descriptors of client and server */
    int serverfd, clientfd;



    //greet message


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
                  &yes, sizeof(int)) == -1) {
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


    socklen_t client_len = sizeof(client);
    while(1){

        clientfd=accept(serverfd,(struct sockaddr *) &client, &client_len);
        if (clientfd == -1) {
            perror("Accept error:");
            return EXIT_FAILURE;
        }

        int *socket_ptr = malloc(sizeof(int));
        *socket_ptr = clientfd;

        pthread_t thread;


        pthread_create(&thread, NULL, myThread, socket_ptr);


    }

    /* Close the local socket */
    close(serverfd);
    libera_lista();

    printf("Connection closed\n\n");

    return EXIT_SUCCESS;
}