#ifndef STRING_BST
#define STRING_BST

#define WBAN_LENGTH 11

typedef struct node {
    char *data;
    char WBAN[WBAN_LENGTH + 1];
    int position;
    struct node *left, *right;
} node;

node *create_node(char *data, char *station_WBAN);
node *BST_insert(node *root, char *data, char *station_WBAN);
node* BST_search(node *root, int position);
void inorder_recursive(node *root);
node *destroy_node(node *n);
node *forest_fire(node *root);

#endif STRING_BST