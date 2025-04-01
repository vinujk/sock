#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>

struct session {
    char sender[16];
    char receiver[16];
    uint8_t tunnel_id;
    time_t last_path_time;
    struct session *next;
};

/* Define path_msg structure */
typedef struct path_msg {
    uint8_t tunnel_id;
    struct in_addr src_ip;
    struct in_addr dest_ip;
    struct in_addr next_hop_ip;
    uint8_t IFH;
    uint8_t time_interval;
    uint8_t setup_priority;
    uint8_t hold_priority;
    uint8_t flags;
    char name[32];
} path_msg;

/* Define resv_msg structure */
typedef struct resv_msg {
    uint8_t tunnel_id;
    struct in_addr src_ip;
    struct in_addr dest_ip;
    struct in_addr next_hop_ip;
    uint8_t IFH;
    uint8_t time_interval;
} resv_msg;

typedef struct dbnode {
    void *data;
    struct dbnode *left, *right;
    int height;
}db_node;


static inline int get_height(db_node *node) {
    return node ? node->height : 0;
}

static inline int max(int a, int b) {
    return (a > b) ? a : b;
}

static inline int get_balance(db_node *node) {
    return node ? get_height(node->left) - get_height(node->right) : 0;
}

typedef int (*cmp)(int, const void *);
//db_node* insert_node(db_node *, path_msg *, cmp func);
db_node* delete_node(db_node *, int, cmp func, int);
//db_node* search_node(db_node *, path_msg *, cmp func);
void free_tree(db_node *);
void display_tree(db_node *);

struct session* insert_session(struct session* , uint8_t, char[], char[]);
struct session* delete_session(struct session* , char[], char[]);
db_node* path_tree_insert(db_node*, char[]);
db_node* resv_tree_insert(db_node*, char[]);
int compare_path_del(int , const void *);
int compare_resv_del(int , const void *);
