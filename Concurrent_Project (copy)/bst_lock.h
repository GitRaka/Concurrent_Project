// Author: Rakesh Kumar
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <bits/stdc++.h>

using namespace std;

// defining a node structure for the binary search tree
typedef struct node
{
    int val;
    int key;
    struct node *left;
    struct node *right;
    pthread_mutex_t rw_lock;
}node_bst;

// the structure for the range functionality
typedef struct Execute
{
    int start;
    int end;
    node_bst *node;
}exec_range;

// the BST functions to be used for the implementation of concurrent BST implementation
node_bst* get_node(node_bst *root, int key);
void get_nodes(node_bst* root, int key_start, int key_end, int tid);
void put_node(node_bst* root, int key, int val, int thrd_num);
void query_range(node_bst* root, int key_start, int key_end, int tid);
void free_bst(node_bst* root);
void disp_tree(node_bst* root);
