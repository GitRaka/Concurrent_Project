// Author: Rakesh Kumar
#include "bst_lock.h"
#include <bits/stdc++.h>

extern node_bst* m_root;
extern vector <exec_range> query[2];
extern pthread_mutex_t lock_bst;

// BST node creation function
node_bst* node_create(int key, int val)
{
    //printf("\nEntered the node creation function.\n");
    node_bst* node = (node_bst*) malloc(sizeof(node_bst));
    // assignt the key and node values passed to the function to the new node
    node->key = key;
    node->val = val;
    node->left = NULL;
    node->right = NULL;

    // initialize the lock of the node
    pthread_mutex_init(&node->rw_lock, NULL);
    return node;
}

// function to update the value of the specified BST node
void val_update(node_bst* node, int val)
{
    printf("\nEntered the node value update function\n");
    node->val = val;
}

// function to insert the new key-val pair into the BST
void put_node(node_bst* root, int key, int val, int thrd_num)
{
    //printf("\nEntered the function that inserts a new key-val pair into the BST.\n");
    if(NULL == root)
    {
        // locking the reader/writer lock
        pthread_mutex_lock(&lock_bst);
        if(NULL == m_root)
        {
            // this is the root node creation condition
            m_root = node_create(key, val);
            printf("\nInitialized Root Node with key: %d, val: %d.\n", m_root->key, m_root->val);
            pthread_mutex_unlock(&lock_bst);
            return;
        }

        pthread_mutex_lock(&m_root->rw_lock);
        root = m_root;
        pthread_mutex_unlock(&lock_bst);
    }

    if(key < (root->key))
    {
        // condition for the insertion to the left on the first node
        if(NULL == root->left)
        {
            // creating a new node for this condition
            root->left = node_create(key, val);
            pthread_mutex_unlock(&root->rw_lock);
        }
        else
        {
            // lock the new node to be added
            pthread_mutex_lock(&root->left->rw_lock);
            // unlock the root node's lock here and hand over pass to the new node added on the left, locking the new node being added
            pthread_mutex_unlock(&root->rw_lock);
            // adding the new node to the leftnow
            put_node(root->left, key, val, thrd_num);
        }        
    }
    else if(key > (root->key))
    {
        // condition to add the new node to the right
        if(root->right == NULL)
        {
            // creating a new node to be inserted to the right for this condition
            root->right = node_create(key, val);
            pthread_mutex_unlock(&root->rw_lock);
        }
        else
        {
            // hand over passing lock to the new right node
            pthread_mutex_lock(&root->right->rw_lock);
            pthread_mutex_unlock(&root->rw_lock);
            put_node(root->right, key, val,thrd_num);
        }
    }
    else
    {
        // this condition is the one where we are trying to update the contents of the root node itself
        if(root->key == key)
        {
            val_update(root, val);
        }
        pthread_mutex_unlock(&root->rw_lock);
    }    
}

// below function fetches the node from the tree based on the key value
node_bst *get_node(node_bst* root, int key)
{
    //printf("\nEntered the function that fetches the node from the tree based on the provided key.\n");
    if(NULL == root)
    {
        pthread_mutex_lock(&lock_bst);
        // empty condition
        if(NULL == m_root)
        {
            pthread_mutex_unlock(&lock_bst);
            return NULL;    // since no element here now
        }
        pthread_mutex_lock(&m_root->rw_lock);
        root = m_root;
        pthread_mutex_unlock(&lock_bst);
    }

    // below condition for the key matching the root's key
    if(root->key == key)
    {
        pthread_mutex_unlock(&root->rw_lock);
        return root;                // since there is a match with the root
    }
    else if(key < (root->key))
    {
        if(NULL == root->left)
        {
            pthread_mutex_unlock(&root->rw_lock);
            return NULL;            // since no element here now
        }
        else
        {
            // locking for the BST node on the left
            pthread_mutex_lock(&root->left->rw_lock);
            // removing the lock on the root node (simulating the lock passing)
            pthread_mutex_unlock(&root->rw_lock);
            // recursive call until key matches
            get_node(root->left, key);
        }        
    }
    else if(key > (root->key))
    {
        if(NULL == root->right)
        {
            pthread_mutex_unlock(&root->rw_lock);
            return NULL;            // since no element here now
        }
        else
        {
            // locking for the BST node on the right
            pthread_mutex_lock(&root->right->rw_lock);
            // unlocking the root node for simulating the hand over hand lock passing
            pthread_mutex_unlock(&root->rw_lock);
            // recursive call until there is a key match on the right
            get_node(root->right, key);
        }
    }
    else
    {
        // handling the invalid scenario
        pthread_mutex_unlock(&lock_bst);
        return NULL;
    }
}

// below function to fetch nodes from the BST based on the provided range to the function
void get_nodes(node_bst* root, int key_start, int key_end, int tid)
{
    //printf("\nEntered the function that fetches the nodes from the BST based on the provided range.\n");
    if(NULL == root)
    {
        // so that no other thread changes the scenario here, we lock below
        pthread_mutex_lock(&lock_bst);
        if(NULL == m_root)
        {
            pthread_mutex_unlock(&lock_bst);        // error scenario since no valid BST
            return;
        }
        pthread_mutex_lock(&m_root->rw_lock);
        root = m_root;                              // assign the main root again for valid next iteration
        pthread_mutex_unlock(&lock_bst);
    }

    if(key_start < root->key)
    {
        // check for valid left nodes to the root for start-key<(root->key)
        if(NULL != root->left)
        {
            // activate the lock on the left of the root node in the BST
            pthread_mutex_lock(&root->left->rw_lock);
            // hand over hand lock pass gesture by unlocking the root->lock below
            pthread_mutex_unlock(&root->rw_lock);
            // recursive call till the match of the key_start
            get_nodes(root->left, key_start, key_end, tid);
            pthread_mutex_lock(&root->rw_lock);             // will get unlocked on the next iteration
        }
    }

    if((key_start <= root->key) && (key_end >= root->key))
    {
        query[tid].push_back({key_start, key_end, root}); // capture values in the vector
    }

    if(key_end > (root->key))
    {
        // check for the valid nodes to the right of the root for end-key > (root->key)
        if(NULL != root->right)
        {
            // activate the lock on the right node of the root node in the BST
            pthread_mutex_lock(&root->right->rw_lock);
            // hand over hand lock pass gesture by unlocking the root node
            pthread_mutex_unlock(&root->rw_lock);
            // recursive call till match for the key_end
            get_nodes(root->right, key_start, key_end, tid);
            // will mostly get unlocked in the next iteration
            pthread_mutex_lock(&root->rw_lock);
        }
    }
    pthread_mutex_unlock(&root->rw_lock);
}

void query_range(node_bst* root, int key_start, int key_end, int tid)
{
    //printf("\nEntered the function to range querrying.\n");
    if(NULL == root)
    {
        return;
    }
    node_bst* node_start = get_node(NULL, key_start);
    node_bst* node_end = get_node(NULL, key_end);
    if(NULL == node_start)
    {
        printf("\nQuery is invalid. Node: %d absent.\n", key_start);
    }
    else if(NULL == node_end)
    {
        printf("\nQuery is invalid. Node: %d absent.\n", key_end);
    }
    else
    {
        pthread_mutex_lock(&lock_bst);
        pthread_mutex_lock(&root->rw_lock);
        pthread_mutex_unlock(&lock_bst);
        // initiate the call to fetch nodes based on the valid range start and end keys
        get_nodes(root, key_start, key_end, tid);
    }
    
}


// below function displays the key-val contents of the BST
void disp_tree(node_bst* root)
{
    //printf("\nEntered the function that will display the elements of the BST.\n");
    if(NULL == root)
    {
        return;                     // since empty BST
    }
    disp_tree(root->left);
    printf("key:%d    val:%d\n", root->key, root->val);
    disp_tree(root->right);
}

// below function does the cleaning and freeing of the BST
void free_bst(node_bst* root)
{
    //printf("\nEntered the function that does the freeing up of the BST.\n");
    if(NULL != root)
    {
        free_bst(root->right);
        free_bst(root->left);
        pthread_mutex_destroy(&root->rw_lock);
        free(root);
    }
}