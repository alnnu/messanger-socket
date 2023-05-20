//Código para compilar e executar:  g++ cliente.cpp -o cliente && ./cliente
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Defines the server port */
#define PORT 4242

/* Sockets buffers length */
#define LEN 4096

#include <pthread.h>

/* Server address */
#define SERVER_ADDR "127.0.0.1"			//o cliente se conecta automaticamente ao servidor no host local (localhost) na porta 4242

/*
 * Main execution of the client program of our simple protocol
 */

/* Receive buffer */
char buffer_in[LEN];
/* Send buffer */
char buffer_out[LEN];


char *msg_server_decoder(char *msg) {
    char *msg_return = (char*) malloc(sizeof (char) * LEN);
    char *aux = strtok(msg, "|");
    int count = 1;

    while(strcmp(aux, "eom") != 0)
    {
        if(count == 2) {
            if(strcmp(aux, "msg_servidor") == 0) {
                count ++;
                aux=strtok(NULL, "|");
                msg_return = aux;
            }
        } else{
            count ++;
            aux=strtok(NULL, "|");
        }
    }
    return msg_return;
}

void *buffer_in_listner(void *ptr) {
    int server = *(int*)ptr;
    char *msg = (char*) malloc(sizeof(char) * LEN);
    while (1){
        recv(server, msg, LEN, 0);
        strcpy(buffer_in, msg_server_decoder(msg));
        printf("%s\n", buffer_in);
        if(strcmp(buffer_in, "bye!") == 0){
            break;
        }
    }


    /* Communicates with the client until bye message come */
    pthread_exit(NULL);
}

char *make_msg(char *type, char *text) {
    char *msg = (char*) malloc(sizeof(char) * LEN);

    strcat(msg,"bom|");
    strcat(msg,type);
    strcat(msg, "|");
    strcat(msg, text);
    strcat(msg, "|eom");

    return msg;
}

int main(int argc, char *argv[]) {
    char apelido[100];

    /* Server socket */
    struct sockaddr_in server;
    /* Client file descriptor for the local socket */
    int sockfd;

    int len = sizeof(server);
    int slen;



    fprintf(stdout, "Starting Client ...\n");

    /*
     * Creates a socket for the client
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error on client socket creation:");
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Client socket created with fd: %d\n", sockfd);

    /* Defines the connection properties */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    memset(server.sin_zero, 0x0, 8);

    /* Tries to connect to the server */
    if (connect(sockfd, (struct sockaddr*) &server, len) == -1) {
        perror("Can't connect to server");
        return EXIT_FAILURE;
    }

    /* Receives the presentation message from the server */
    if ((slen = recv(sockfd, buffer_in, LEN, 0)) > 0) {
        buffer_in[slen + 1] = '\0';
        fprintf(stdout, "Server says: %s\n", msg_server_decoder(buffer_in));
    }

    fprintf(stdout, "apelido: ");
    fgets(apelido, 100, stdin);

    //remover o enter
    apelido[strlen(apelido) - 1] = '\0';

    strcpy(buffer_out,apelido);


    /* Sends the read message to the server through the socket */
    send(sockfd, make_msg("usuario_entra", buffer_out), LEN, 0);

    /*
     * Communicate with the server until the exit message come
     */

    int *socket_ptr = malloc(sizeof(int));
    *socket_ptr = sockfd;

    pthread_t thread;


    pthread_create(&thread, NULL, buffer_in_listner, socket_ptr);
    fprintf(stdout, "Say something to the server:\n");

    while (true) {

        /* Zeroing the buffers */
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);

        fgets(buffer_out, LEN, stdin);
        buffer_out[strlen(buffer_out) - 1] = '\0';

        /* Sends the read message to the server through the socket */
        if(strcmp(buffer_out, "bye") == 0){
            send(sockfd, make_msg("usuario_sai",apelido), LEN, 0);

        } else {
            send(sockfd,  make_msg("msg_cliente",buffer_out), LEN, 0);
        }

        /* 'bye' message finishes the connection */
        if(strcmp(buffer_out, "bye") == 0)
            break;
    }

    /* Close the connection with the server */
    close(sockfd);

    fprintf(stdout, "\nConnection closed\n\n");

    return EXIT_SUCCESS;
}
