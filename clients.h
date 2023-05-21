
struct client{
    int fd;
    char *nome;
    struct client *prox;
};

typedef struct client* Lista;

Lista* create_clients_list();
int tamanho_lista();
void libera_lista();
int add(int, char*);
int remover(int);
int get_client_fd(int);
struct client *get_client_pos(int);
