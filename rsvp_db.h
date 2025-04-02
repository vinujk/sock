#include<stdio.h>
#include<string.h>
#include<stdlib.h>


struct session {
    char sender[16];
    char receiver[16];
    uint8_t tunnel_id;
    time_t last_path_time;
    struct session *next;
};

/* Define path_msg structure */
typedef struct path_msg {
    struct in_addr src_ip;
    struct in_addr dest_ip;
    struct in_addr nexthop_ip;
    uint8_t tunnel_id;
    uint8_t IFH;
    uint8_t interval;
    uint8_t setup_priority;
    uint8_t hold_priority;
    uint8_t flags;
    uint16_t lsp_id;
    char name[16];
} path_msg;

/* Define resv_msg structure */
typedef struct resv_msg {
    struct in_addr src_ip;
    struct in_addr dest_ip;
    struct in_addr nexthop_ip;
    uint8_t tunnel_id;
    uint8_t IFH;
    uint8_t interval;
    uint32_t in_label;
    uint32_t out_label;
    uint16_t lsp_id
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

struct session* insert_session(struct session*, uint8_t, char[], char[], uint8_t);
struct session* delete_session(struct session*, char[], char[]);
db_node* path_tree_insert(db_node*, char[]);
db_node* resv_tree_insert(db_node*, char[]);
int compare_path_del(int , const void *);
int compare_resv_del(int , const void *);

