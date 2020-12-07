
// Author: Rakesh Kumar
#include "handler.h"
#include "bst_lock.h"
#include "bst_lock_rw.h"
//#include <algorithm>
#define THREAD_MAX (10)

vector <exec_range> query[2];
vector <rwexec_range> rwquery[2];

struct info_thrd
{
    int tsk_id;
    int tsk_key;
    int tsk_len;
    char fil_name[64];
};

node_bst *m_root;
rwnode_bst *rwm_root;

pthread_rwlock_t rwlock_bst;
pthread_mutex_t lock_bst;


struct timespec t_begin, t_end;

void *rwhandler_put(void *arg)
{
    printf("\nEntered the put handler callback function for the RW lock.\n");
    struct info_thrd *thrd_info = (struct info_thrd *)arg;
    int key=0; int val=0; int i=0;
    FILE *fil = fopen(thrd_info->fil_name, "r");

    if(NULL == fil)
    {
        printf("Error in opening the file.");
        exit(0);
    }

    while((!feof(fil)) && (i<thrd_info->tsk_key))
    {
        fscanf(fil, "%d  %d\n", &key, &val);
        i++;
    }

    for(i=0; i<(thrd_info->tsk_len);i++)
    {
        fscanf(fil, "%d  %d\n", &key, &val);
        rwput_node(NULL, key, val, thrd_info->tsk_id);
    }
    fclose(fil);
    return 0;
}

void *rwhandler_get(void *arg)
{
    printf("\nEntered the get handler calback function for the RW lock\n");
    struct info_thrd* thrd_info = (struct info_thrd*) arg;
    rwnode_bst* node;
    int i=0; int key=0;
    FILE *fil = fopen(thrd_info->fil_name, "r");
    if(NULL == fil)
    {
        printf("\nFile opening error.\n");
        exit(0);
    }

    while((!feof(fil)) && (i<thrd_info->tsk_key))
    {
        fscanf(fil, "%d\n", &key);
        i++;
    }

    for(i=0; i<(thrd_info->tsk_len); i++)
    {
        fscanf(fil, "%d\n", &key);
        node = rwget_node(NULL, key);
        if(NULL == node)
        {
            printf("\nNode with key: %d could not be found.\n", key);
        }
        else
        {
            printf("\nSearch successful, key: %d contains the value: %d\n", node->key, node->val);
        }
        node = NULL;
    }
    fclose(fil);
    return 0;
}

void *rwhhandler_range(void *arg)
{
    printf("\nEntered the range handler callback function.\n");
    struct info_thrd* thrd_info = (struct info_thrd*) arg;
    int key_start;
    int key_end;
    int i=0;
    FILE *fil = fopen(thrd_info->fil_name, "r");
    if(NULL == fil)
    {
        printf("\nFile opening error in the range handler function.\n");
        exit(0);
    }

    while((!feof(fil)) && (i<thrd_info->tsk_id))
    {
        fscanf(fil, "%d  %d\n", &key_start, &key_end);
        i++;
    }

    for(i=0; i<(thrd_info->tsk_len); i++)
    {
        fscanf(fil, "%d  %d\n", &key_start, &key_end);
        if(key_end > key_start)
        {
            rwget_nodes(NULL, key_start, key_end, thrd_info->tsk_id);
        }
    }
    fclose(fil);
    return 0;
}

// thread callback function to handle the put operation
void *handler_put(void *arg)
{
    printf("\nEntered the thread handler function for the BST put function.\n");
    struct info_thrd *thrd_info = (struct info_thrd *) arg;
    int value = 0;
    int key = 0;
    int i = 0;

    FILE* fil_ptr = fopen(thrd_info->fil_name, "r");
    if(NULL == fil_ptr)
    {
        printf("\nFile opening error encounterd in the put function handler.\n");
        exit(0);
    }

    while((!feof(fil_ptr)) && (i < thrd_info->tsk_key))
    {
        fscanf(fil_ptr, "%d %d\n", &key, &value);
        i++;
    }

    for(i=0; i<thrd_info->tsk_len; i++)
    {
        fscanf(fil_ptr, "%d %d\n", &key, &value);
        put_node(NULL, key, value, thrd_info->tsk_id);
    }
    fclose(fil_ptr);
    return 0;
}

// thread callback function to handle the get operation on the BST
void *handler_get(void *arg)
{
    printf("\nEntered the thread handler function for the get operation on the BST.\n");
    struct info_thrd* thrd_info = (struct info_thrd *) arg;
    node_bst* node;
    int key = 0;
    int i = 0;
    FILE *fil_ptr = fopen(thrd_info->fil_name, "r");

    if(NULL == fil_ptr)
    {
        printf("\nFile opening error encountered in the get function thread handler\n");
        exit(0);
    }

    while((!feof(fil_ptr)) && (i<thrd_info->tsk_key))
    {
        fscanf(fil_ptr, "%d\n", &key);
        i++;
    }

    for(i=0; i< (thrd_info->tsk_len); i++)
    {
        fscanf(fil_ptr, "%d\n", &key);
        node = get_node(NULL, key);
        if(NULL == node)
        {
            printf("\nNode with key: %d not found.\n", key);
        }
        else
        {
            printf("\nSearch successful. Node with key: %d has the value: %d.\n", node->key, node->val);
        }
        node = NULL;        
    }
    fclose(fil_ptr);
    return 0;
}

// thread callback function to handle the range query operation on the BST
void *handler_range(void *arg)
{
    printf("\nEntered the thread handler callback function for the range_query operation on the BST.\n");
    struct info_thrd* thrd_info = (struct info_thrd *) arg;
    int key_start = 0;
    int key_end = 0;
    int i=0;
    FILE *file_ptr = fopen(thrd_info->fil_name, "r");
    if(NULL == file_ptr)
    {
        printf("\nFile opening error encountered in the range_handler function thread handler.\n");
        exit(0);
    }
    while((feof(file_ptr)) && (i<thrd_info->tsk_key))
    {
        fscanf(file_ptr, "%d %d\n", &key_start, &key_end);
        i++;
    }

    for(i=0; i<thrd_info->tsk_len; i++)
    {
        fscanf(file_ptr, "%d %d\n", &key_start, &key_end);
        if(key_end > key_start)
        {
            get_nodes(NULL, key_start, key_end, thrd_info->tsk_id);
        }
    }
    fclose(file_ptr);
    return 0;
}

int main(int argc, char **argv)
{
    printf("\nEntered the main function.\n");
    struct parm_handler parm;
    int i = 0;
    parser(argc, argv, &parm);
    printf("\nReturned from the parser in the main.\n");
    
    if(parm.num_ins_keys < parm.tot_thrds)
    {
        parm.tot_thrds = (parm.num_ins_keys / 2);
    }

    if(parm.tot_thrds < 6)
    {
        parm.tot_thrds = 6;
    }

    struct info_thrd thrd_info[parm.tot_thrds];

    printf("\nparm.tot_thrds: %d\n", parm.tot_thrds);
    pthread_t thrd[parm.tot_thrds];

    int len = (parm.num_ins_keys / (parm.tot_thrds - 4));
    int m=0;
    int j=0;

    // current code block for BST main flow
    printf("\nStarting the current code block.\n");
    if (parm.lock_rw_set != true)
    {
        printf("\nEntered the mutex lock scenario.\n");
        pthread_mutex_init(&lock_bst, NULL);
        clock_gettime(CLOCK_MONOTONIC, &t_begin);

        for(i=0; i<(parm.tot_thrds - 4); i++)
        {
            printf("The i value in the for loop: %d", i);
            printf("\nInside the main for loop for thread creation.\n");
            m = i*len;
            strcpy(thrd_info[i].fil_name, parm.in_file);
            printf("\nCopy completed.\n");

            if((parm.tot_thrds - 5) == i)
            {
                printf("\nInside thrd_info if condition.\n");
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = (parm.num_ins_keys - m);
                thrd_info[i].tsk_key = m;
            }
            else
            {
                printf("\nInside thrd_info else condition.\n");
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = len;
                thrd_info[i].tsk_key = m;
            }
            
            printf("\nBefore the thread create function.\n");

            // int ret_thrd = pthread_create(&thrd[i], NULL, handler_put, (void *)&thrd_info[i]);
            // printf("\nThe return from the pthread_)create call for handler_put callback is: %d\n", ret_thrd);

            // calling the threads for the put functionality
            if(pthread_create(&thrd[i], NULL, handler_put, (void *)&thrd_info[i]) != 0)
            {
                printf("\nThread creation error.\n");
                exit(0);
            }
            else
            {
                printf("\nThread with id: %d created.\n", thrd_info[i].tsk_id);
            }

            printf("\nThread creation block completed.\n");
        }
        
        len = (parm.num_srch_keys/2);
        m=0;j=0;

        for(; i<(parm.tot_thrds - 2); i++)
        {
            strcpy(thrd_info[i].fil_name, parm.srch_key_file);
            m = len*j;
            if((parm.tot_thrds-3) == i)
            {
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = (parm.num_srch_keys - m);
                thrd_info[i].tsk_key = m;
            }
            else
            {
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = len;
                thrd_info[i].tsk_key = m;
            }
            //int ret = pthread_create(&thrd[i], NULL, handler_get, (void *)&thrd_info[i]);
            if(0 != pthread_create(&thrd[i], NULL, handler_get, (void *)&thrd_info[i]))
            {
                printf("\nThread creation error.\n");
                exit(0);
            }
            else
            {
                printf("\nThread with id: %d created.\n", thrd_info[i].tsk_id);
            }
            j++;
        }
        
        len=(parm.rng_queries/2);
        m=0;j=0;
        for(; i<(parm.tot_thrds); i++)
        {
            strcpy(thrd_info[i].fil_name, parm.rng_file);
            m=len*j;

            if((parm.tot_thrds - 1) == i)
            {
                thrd_info[i].tsk_id = j;
                thrd_info[i].tsk_len = (parm.rng_queries - m);
                thrd_info[i].tsk_key = m;
            }
            else
            {
                thrd_info[i].tsk_id = j;
                thrd_info[i].tsk_len = len;
                thrd_info[i].tsk_key = m;
            }

            if(0 != (pthread_create(&thrd[i], NULL, handler_range, (void *)&thrd_info[i])))
            {
                printf("\nThread creation error.\n");
                exit(0);
            }
            else
            {
                printf("\nThread with id: %d created.\n", thrd_info[i].tsk_id);
            }
            j++;            
        }

        for(i=0; i<parm.tot_thrds; i++)
        {
            pthread_join(thrd[i], NULL);
        }
        
        clock_gettime(CLOCK_MONOTONIC, &t_end);
        pthread_mutex_destroy(&lock_bst);

        printf("\nResult for the range query:\n");
        for(i=0; i<2; i++)
        {
            for(j=0; j<query[i].size(); j++)
            {
                //printf("Query for the range by thread: %d for %d to %d = %d\n", i, query[i][j].start, query[i][j].end, query[i][j].node->key);
            }
        }
        printf("\nThe sorted BST output:\n");
        disp_tree(m_root);
        free_bst(m_root);

        unsigned long long passed_ns;
        passed_ns = (t_end.tv_sec - t_begin.tv_sec)*1000000000 + (t_end.tv_nsec-t_begin.tv_nsec);
        printf("\nPassed time in (ns): %llu\n", passed_ns);
        double pasd_s = ((double)passed_ns) / 1000000000.0;
        printf("\nPassed time in (s): %lf\n", pasd_s);
    }
    else
    {
        printf("\nEntered the read-write lock scenario.\n");
        pthread_rwlock_init(&rwlock_bst, NULL);
        clock_gettime(CLOCK_MONOTONIC, &t_begin);

        for(i=0; i<(parm.tot_thrds - 4); i++)
        {
            m = len*i;
            strcpy(thrd_info[i].fil_name, parm.in_file);

            if((parm.tot_thrds - 5) == i)
            {
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = (parm.num_ins_keys - m);
                thrd_info[i].tsk_key = m;
            }
            else
            {
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = len;
                thrd_info[i].tsk_key = m;
            }
            
            if(0 != (pthread_create(&thrd[i], NULL, rwhandler_put, (void*)&thrd_info[i])))
            {
                printf("\nThread creation error.\n");
                exit(0);
            }
            else
            {
                printf("\nThread with id: %d created.\n", thrd_info[i].tsk_id);
            }            
        }
        len = (parm.num_srch_keys/2);
        m=0; j=0;

        for(; i<(parm.tot_thrds - 2); i++)
        {
            strcpy(thrd_info[i].fil_name, parm.srch_key_file);
            m=len*j;
            if((parm.tot_thrds - 3) == i)
            {
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = (parm.num_srch_keys - m);
                thrd_info[i].tsk_key = m;
            }
            else
            {
                thrd_info[i].tsk_id = i;
                thrd_info[i].tsk_len = len;
                thrd_info[i].tsk_key = m;
            }

            if(0 != (pthread_create(&thrd[i], NULL, rwhandler_get, (void*)&thrd_info[i])))
            {
                printf("\nThread creation error\n");
                exit(0);
            }
            else
            {
                printf("\nGet thread with thread_id: %d created.\n", thrd_info[i].tsk_id);
            }
            j++;            
        }

        len = (parm.rng_queries/2);
        m=0; j=0;
        for(; i<(parm.tot_thrds); i++)
        {
            strcpy(thrd_info[i].fil_name, parm.rng_file);
            m = len*j;

            if((parm.tot_thrds-1) == i)
            {
                thrd_info[i].tsk_id = j;
                thrd_info[i].tsk_len = (parm.rng_queries - m);
                thrd_info[i].tsk_key = m;
            }
            else
            {
                thrd_info[i].tsk_id = j;
                thrd_info[i].tsk_len = len;
                thrd_info[i].tsk_key = m;
            }

            if(0 != pthread_create(&thrd[i], NULL, rwhhandler_range, (void*)&thrd_info[i]))
            {
                printf("\nThread creation error.\n");
                exit(0);
            }
            else
            {
                printf("\nThread for range handler with id: %d created.\n", thrd_info[i].tsk_id);
            }
            j++;
        }

        for(i = 0; i<(parm.tot_thrds); i++)
        {
            pthread_join(thrd[i], NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &t_end);
        pthread_rwlock_destroy(&rwlock_bst);

        printf("\nResult for the range query:\n");
        for(i=0; i<2; i++)
        {
            for(j=0; j<(rwquery[i].size()); j++)
            {
                //printf("\nQuery for range by thread id: %d for %d to %d = %d\n", i, rwquery[i][j].key_start, rwquery[i][j].key_end, rwquery[i][j].node->key);
            }
        }

        printf("\nBST in order.\n");
        rwdisp_tree(rwm_root);
        rwfree_bst(rwm_root);

        unsigned long long passed_ns;
        passed_ns = (t_end.tv_sec - t_begin.tv_sec)*1000000000 + (t_end.tv_nsec-t_begin.tv_nsec);
        printf("\nPassed time in (ns): %llu\n", passed_ns);
        double pasd_s = ((double)passed_ns) / 1000000000.0;
        printf("\nPassed time in (s): %lf\n", pasd_s);
    }        
    printf("\n\n");
    return 0;
}