// Author: Rakesh Kumar
#include "bst_lock_rw.h"
#include <bits/stdc++.h>

extern rwnode_bst* rwm_root;
extern vector <rwexec_range> rwquery[2];
extern pthread_rwlock_t rwlock_bst;

// BST node creation function
rwnode_bst* rwnode_create(int key, int val)
{
    //printf("\nEntered the node creation function.\n");
    rwnode_bst* node = (rwnode_bst*) malloc(sizeof(rwnode_bst));
    // assignt the key and node values passed to the function to the new node
    node->key = key;
    node->val = val;
    node->left = NULL;
    node->right = NULL;

    // initialize the lock of the node
    pthread_rwlock_init(&node->lck, NULL);
    return node;
}

// function to update the value of the specified BST node
void val_update(rwnode_bst* node, int val)
{
    printf("\nEntered the node value update function\n");
    node->val = val;
}

// function to insert the new key-val pair into the BST
void rwput_node(rwnode_bst* root, int key, int val, int thrd_num)
{
    //printf("\nEntered the function that inserts a new key-val pair into the BST.\n");
    if(NULL == root)
    {
        // locking the reader/writer lock
        pthread_rwlock_wrlock(&rwlock_bst);
        if(NULL == rwm_root)
        {
            // this is the root node creation condition
            rwm_root = rwnode_create(key, val);
            printf("\nInitialized Root Node with key: %d, val: %d.\n", rwm_root->key, rwm_root->val);
            pthread_rwlock_unlock(&rwlock_bst);
            return;
        }

        pthread_rwlock_wrlock(&rwm_root->lck);
        root = rwm_root;
        pthread_rwlock_unlock(&rwlock_bst);
    }

    if(key < (root->key))
    {
        // condition for the insertion to the left on the first node
        if(NULL == root->left)
        {
            // creating a new node for this condition
            root->left = rwnode_create(key, val);
            pthread_rwlock_unlock(&root->lck);
        }
        else
        {
            // lock the new node to be added
            pthread_rwlock_wrlock(&root->left->lck);
            // unlock the root node's lock here and hand over pass to the new node added on the left, locking the new node being added
            pthread_rwlock_unlock(&root->lck);
            // adding the new node to the leftnow
            rwput_node(root->left, key, val, thrd_num);
        }        
    }
    else if(key > (root->key))
    {
        // condition to add the new node to the right
        if(root->right == NULL)
        {
            // creating a new node to be inserted to the right for this condition
            root->right = rwnode_create(key, val);
            pthread_rwlock_unlock(&root->lck);
        }
        else
        {
            // hand over passing lock to the new right node
            pthread_rwlock_wrlock(&root->right->lck);
            pthread_rwlock_unlock(&root->lck);
            rwput_node(root->right, key, val,thrd_num);
        }
    }
    else
    {
        // this condition is the one where we are trying to update the contents of the root node itself
        if(root->key == key)
        {
            val_update(root, val);
        }
        pthread_rwlock_unlock(&root->lck);
    }    
}

// below function fetches the node from the tree based on the key value
rwnode_bst *rwget_node(rwnode_bst* root, int key)
{
    //printf("\nEntered the function that fetches the node from the tree based on the provided key.\n");
    if(NULL == root)
    {
        pthread_rwlock_rdlock(&rwlock_bst);
        // empty condition
        if(NULL == rwm_root)
        {
            pthread_rwlock_unlock(&rwlock_bst);
            return NULL;    // since no element here now
        }
        pthread_rwlock_rdlock(&rwm_root->lck);
        root = rwm_root;
        pthread_rwlock_unlock(&rwlock_bst);
    }

    // below condition for the key matching the root's key
    if(root->key == key)
    {
        pthread_rwlock_unlock(&root->lck);
        return root;                // since there is a match with the root
    }
    else if(key < (root->key))
    {
        if(NULL == root->left)
        {
            pthread_rwlock_unlock(&root->lck);
            return NULL;            // since no element here now
        }
        else
        {
            // locking for the BST node on the left
            pthread_rwlock_rdlock(&root->left->lck);
            // removing the lock on the root node (simulating the lock passing)
            pthread_rwlock_unlock(&root->lck);
            // recursive call until key matches
            rwget_node(root->left, key);
        }        
    }
    else if(key > (root->key))
    {
        if(NULL == root->right)
        {
            pthread_rwlock_unlock(&root->lck);
            return NULL;            // since no element here now
        }
        else
        {
            // locking for the BST node on the right
            pthread_rwlock_rdlock(&root->right->lck);
            // unlocking the root node for simulating the hand over hand lock passing
            pthread_rwlock_unlock(&root->lck);
            // recursive call until there is a key match on the right
            rwget_node(root->right, key);
        }
    }
    else
    {
        // handling the invalid scenario
        pthread_rwlock_unlock(&rwlock_bst);
        return NULL;
    }
}

// below function to fetch nodes from the BST based on the provided range to the function
void rwget_nodes(rwnode_bst* root, int key_start, int key_end, int tid)
{
    //printf("\nEntered the function that fetches the nodes from the BST based on the provided range.\n");
    if(NULL == root)
    {
        // so that no other thread changes the scenario here, we lock below
        pthread_rwlock_rdlock(&rwlock_bst);
        if(NULL == rwm_root)
        {
            pthread_rwlock_unlock(&rwlock_bst);        // error scenario since no valid BST
            return;
        }
        pthread_rwlock_rdlock(&rwm_root->lck);
        root = rwm_root;                              // assign the main root again for valid next iteration
        pthread_rwlock_unlock(&rwlock_bst);
    }

    if(key_start < root->key)
    {
        // check for valid left nodes to the root for start-key<(root->key)
        if(NULL != root->left)
        {
            // activate the lock on the left of the root node in the BST
            pthread_rwlock_rdlock(&root->left->lck);
            // hand over hand lock pass gesture by unlocking the root->lock below
            pthread_rwlock_unlock(&root->lck);
            // recursive call till the match of the key_start
            rwget_nodes(root->left, key_start, key_end, tid);
            pthread_rwlock_rdlock(&root->lck);             // will get unlocked on the next iteration
        }
    }

    if((key_start <= root->key) && (key_end >= root->key))
    {
        rwquery[tid].push_back({key_start, key_end, root}); // capture values in the vector
    }

    if(key_end > (root->key))
    {
        // check for the valid nodes to the right of the root for end-key > (root->key)
        if(NULL != root->right)
        {
            // activate the lock on the right node of the root node in the BST
            pthread_rwlock_rdlock(&root->right->lck);
            // hand over hand lock pass gesture by unlocking the root node
            pthread_rwlock_unlock(&root->lck);
            // recursive call till match for the key_end
            rwget_nodes(root->right, key_start, key_end, tid);
            // will mostly get unlocked in the next iteration
            pthread_rwlock_rdlock(&root->lck);
        }
    }
    pthread_rwlock_unlock(&root->lck);
}

void rwquery_range(rwnode_bst* root, int key_start, int key_end, int tid)
{
    //printf("\nEntered the function to range querrying.\n");
    if(NULL == root)
    {
        return;
    }
    rwnode_bst* node_start = rwget_node(NULL, key_start);
    rwnode_bst* node_end = rwget_node(NULL, key_end);
    if(NULL == node_start)
    {
        printf("\nQuery is invalid. Node: %d absent.\n", key_start);
        return;
    }
    else if(NULL == node_end)
    {
        printf("\nQuery is invalid. Node: %d absent.\n", key_end);
        return;
    }
    else
    {
        pthread_rwlock_rdlock(&rwlock_bst);
        pthread_rwlock_rdlock(&root->lck);
        pthread_rwlock_unlock(&rwlock_bst);
        // initiate the call to fetch nodes based on the valid range start and end keys
        rwget_nodes(root, key_start, key_end, tid);
    }
}


// below function displays the key-val contents of the BST
void rwdisp_tree(rwnode_bst* root)
{
    //printf("\nEntered the function that will display the elements of the BST.\n");
    if(NULL == root)
    {
        return;                     // since empty BST
    }
    rwdisp_tree(root->left);
    printf("key:%d    val:%d\n", root->key, root->val);
    rwdisp_tree(root->right);
}

// below function does the cleaning and freeing of the BST
void rwfree_bst(rwnode_bst* root)
{
    //printf("\nEntered the function that does the freeing up of the BST.\n");
    if(NULL != root)
    {
        rwfree_bst(root->right);
        rwfree_bst(root->left);
        pthread_rwlock_destroy(&root->lck);
        free(root);
    }
}