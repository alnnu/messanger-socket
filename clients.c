#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clients.h"


typedef struct client Elem;
Lista* li;

Lista* create_clients_list(){
    li = (Lista*) malloc(sizeof(Lista));
    if(li != NULL)
        *li = NULL;
    return li;
}

void libera_lista(){
    if(li != NULL){
        Elem* no;
        while((*li) != NULL){
            no = *li;
            *li = (*li)->prox;
            free(no);
        }
        free(li);
    }
}


int add(int fd_client, char *nome){
    if(li == NULL)
        return 0;
    Elem *no;
    no = (Elem*) malloc(sizeof(Elem));
    if(no == NULL)
        return 0;
    no->fd = fd_client;
    no->nome = (char *) malloc(sizeof (char) * strlen(nome));
    strcpy(no->nome,nome);
    no->prox = NULL;
    if((*li) == NULL){//lista vazia: insere in�cio
        *li = no;
    }else{
        Elem *aux;
        aux = *li;
        while(aux->prox != NULL){
            aux = aux->prox;
        }
        aux->prox = no;
    }
    return 1;
}


int remover(int fd){
    if(li == NULL)
        return 0;
    if((*li) == NULL)//lista vazia
        return 0;
    Elem *ant, *no = *li;
    while(no != NULL && no->fd != fd){
        ant = no;
        no = no->prox;
    }
    if(no == NULL)//n�o encontrado
        return 0;

    if(no == *li)//remover o primeiro?
        *li = no->prox;
    else
        ant->prox = no->prox;
    free(no);
    return 1;
}

int tamanho_lista(){
    if(li == NULL)
        return 0;
    int cont = 0;
    Elem* no = *li;
    while(no != NULL){
        cont++;
        no = no->prox;
    }
    return cont;
}

struct client *get_client(int fd_client){
    if(li == NULL)
        return NULL;
    Elem *no = *li;
    while(no != NULL && no->fd != fd_client){
        no = no->prox;
    }
    if(no == NULL)
        return NULL;
    else{
        return no;
    }
}