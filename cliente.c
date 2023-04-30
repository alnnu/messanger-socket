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

/* Server address */
#define SERVER_ADDR "127.0.0.1"			//o cliente se conecta automaticamente ao servidor no host local (localhost) na porta 4242


char *msg_servidor(char buffer[LEN]) {
    char *msg;
    char *aux = strtok(buffer, "|");
    int count = 1;

    while(strcmp(aux, "eom") != 0)
    {
        if(count == 2) {
            if(strcmp(aux, "msg_servidor") == 0) {
                count ++;
                aux=strtok(NULL, "|");
                fprintf(stdout, "Server says: %s\n", aux);
                msg = aux;
            }
        } else{
            count ++;
            aux=strtok(NULL, "|");
        }
    }
    return &(*msg);
}

/*
 * Main execution of the client program of our simple protocol
 */
int main(int argc, char *argv[]) {

    /* Server socket */
    struct sockaddr_in server;
    /* Client file descriptor for the local socket */
    int sockfd;

    int len = sizeof(server);
    int slen;

    /* Receive buffer */
    char buffer_in[LEN];
    /* Send buffer */
    char buffer_out[LEN];

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
    memset(server.sin_zero, 0x0, 8);

    /* Tries to connect to the server */
    if (connect(sockfd, (struct sockaddr*) &server, len) == -1) {
        perror("Can't connect to server");
        return EXIT_FAILURE;
    }

    /* Receives the presentation message from the server */

    if ((slen = recv(sockfd, buffer_in, LEN, 0)) > 0) {
        msg_servidor(buffer_in);
    }

    memset(buffer_out, 0x0, 8);
    fprintf(stdout, "apelido: ");
    fgets(buffer_out, LEN, stdin);

    buffer_out[strlen(buffer_out) - 1] = '\0';

    char *aux = (char *)malloc(sizeof(char) * LEN);

    strcat(aux,"bom|usuario_entra|");
    strcat(aux, buffer_out);
    strcat(aux, "|eom");

    strcpy(buffer_out, aux);
    free(aux);


    send(sockfd, buffer_out, LEN, 0);

    recv(sockfd, buffer_in, LEN, 0);

    msg_servidor(buffer_in);
    /*
     * Communicate with the server until the exit message come
     */
    while (true) {
        /* Receives an answer from the server */


        /* Zeroing the buffers */
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);

        fprintf(stdout, "Say something to the server: ");
        fgets(buffer_out, LEN, stdin);

        /* Sends the read message to the server through the socket */
        send(sockfd, buffer_out, strlen(buffer_out), 0);

        slen = recv(sockfd, buffer_in, LEN, 0);

        strcpy(buffer_in, msg_servidor(buffer_in));

        /* 'bye' message finishes the connection */

        if(strcmp(buffer_in, "bye!") == 0)
            break;
    }

    /* Close the connection with the server */
    close(sockfd);

    fprintf(stdout, "\nConnection closed\n\n");

    return EXIT_SUCCESS;
}
