#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "StringBST.h"

static void inorder_recursive_helper(node *root, int *counter);

// start a new BST or create a new node to add to an existing one
node *create_node(char *data, char *WBAN) {
    node *n = calloc(1, sizeof(node));
	if (!n) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

    n->data = calloc(strlen(data) + 1, sizeof(char));
	if (!n->data) {
		printf("Ran out of memory!\n");
		exit(EXIT_FAILURE);
	}

    strcpy(n->data, data);
    strcpy(n->WBAN, WBAN);
	n->left = NULL;
	n->right = NULL;

    return n;
}

// recursively insert a node
node *BST_insert(node *root, char *data, char *WBAN) {
    if (data == NULL)
        return root;

    // if at the location or no nodes exist, return the pointer to a newly created node
    if (root == NULL)
        return create_node(data, WBAN);

    // based on listyCmp (similar to strcmp()) insert to the left or right relative to the current node
    if (strcmp(data, root->data) < 0)
        root->left = BST_insert(root->left, data, WBAN);
    else
        root->right = BST_insert(root->right, data, WBAN);

    return root;
}

// recursively search for a node given its position
node *BST_search(node *root, int position) {
    if (root == NULL)
        return NULL;

    if (root->position == position)
        return root;

    if (position < root->position)
        root = BST_search(root->left, position);
    else
        root = BST_search(root->right, position);

    return root;
}

// inorder traversal with a counter
void inorder_recursive(node *root) {
    int counter = 0;
    inorder_recursive_helper(root, &counter);
}

// inorder traversal and ordered numbering of nodes
static void inorder_recursive_helper(node *root, int *counter) {
    if (root == NULL)
        return;

    // left first
    inorder_recursive_helper(root->left, counter);

    // increment counter and assign to the node's position field
    // print the ListyString and its position
    *counter += 1;
    printf("%3d -", *counter);
    printf("%s", root->data);
    printf("\n");
    root->position = *counter;

    // right second
    inorder_recursive_helper(root->right, counter);
}

node *destroy_node(node *n) {
	if (!n)
		return NULL;

    free(n->data);
    n->data = NULL;
    free(n);
    return NULL;
}

node *forest_fire(node *root) {
    if (!root)
        return NULL;

    if (root->left)
        forest_fire(root->left);

    if (root->right)
        forest_fire(root->right);

    destroy_node(root);
    return NULL;
}
