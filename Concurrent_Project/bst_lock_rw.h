#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <iostream>

using namespace std;

// structure for the node with rw lock
typedef struct rw_node
{
    pthread_rwlock_t lck;
    int key;
    int val;
    struct rw_node* left;
    struct rw_node* right;
}rwnode_bst;

typedef struct rw_test
{
    int key_start;
    int key_end;
    rwnode_bst* node;
}rwexec_range;

rwnode_bst* rwget_node(rwnode_bst* root, int key);
void rwget_nodes(rwnode_bst* root, int key_start, int key_end, int tid);
void rwput_node(rwnode_bst* root, int key, int val, int thrd_num);
void rwquery_range(rwnode_bst* root, int key_start, int key_end, int tid);
void rwfree_bst(rwnode_bst* root);
void rwdisp_tree(rwnode_bst* root);
