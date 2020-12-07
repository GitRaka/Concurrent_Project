// Author: Rakesh Kumar
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include "handler.h"

// function to check valid extension of the file
bool check_textfile(char *argv)
{
    printf("Entered the function that checks for valid file format(txt).");
    int len = strlen(argv);
    char fil_name[len];
    char *token = strtok(fil_name, ".txt");

    if(0 == (strcmp(token, argv)));
    {
        return false;
    }
    return true;
}

// function to get the count of the number of elements in the file
int get_cnt_tot(char *fil_name)
{
    printf("\nFunction to calculate the number of elements from the file.\n");
    char *buf = NULL;
    FILE *f_ptr = NULL;
    size_t tmp = 0;
    int cnt_elmnt = 0;

    f_ptr = fopen(fil_name, "r");
    if(NULL == f_ptr)
    {
        printf("\nFile opening failure.\n");
        return -1;
    }
    else
    {
        printf("\nFile opened successfully.\n");
    }
    

    while(-1 != (getline(&buf, &tmp, f_ptr)))
    {
        cnt_elmnt++;
    }

    free(buf);
    fclose(f_ptr);
    return cnt_elmnt;
}

// function that carries out the parsing of the arguments
int parser(int argc, char **argv, struct parm_handler *parm_handler_t)
{
    printf("\nEnter the argument parser function that handles arguments pertinently.\n");
    char lck[256]; char c;
    static struct option opt[] =
    {
        {"input", required_argument, 0, 'i'},
        {"thread", optional_argument, 0, 't'},
        {"find", required_argument, 0, 'f'},
        {"range", required_argument, 0, 'r'},
        {"lock", required_argument, 0, 'l'},
        {"help", optional_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    parm_handler_t->lock_rw_set = false;
    
    while(-1 != (c = getopt_long(argc, argv, "hi:f:t:r:", opt, 0)))
    {
        switch(c)
        {
            case 'h':
                printf("\nThe usage: ./bst -i [filename] -f [find filename] -r [range-query file] -t [Total threads] --lock=[lock type]\n");
                exit(0);
                break;

            case 'i':
                parm_handler_t->in_file = optarg;
                break;

            case 't':
                parm_handler_t->tot_thrds = atoi(optarg);
                break;

            case 'l':
                strcpy(lck, optarg);
                if(0 == strcmp(lck, "rw_lock"))
                {
                    parm_handler_t->lock_rw_set = 1;
                }
                else
                {
                    parm_handler_t->lock_rw_set = 0;
                }
                break;

            case 'f':
                parm_handler_t->srch_key_file = optarg;
                break;

            case 'r':
                parm_handler_t->rng_file = optarg;
                break;

            default:
                printf("\nArguments absent.\n");
                return -1;
        }
    }

    parm_handler_t->num_ins_keys = get_cnt_tot(parm_handler_t->in_file);
    parm_handler_t->rng_queries = get_cnt_tot(parm_handler_t->rng_file);
    parm_handler_t->num_srch_keys = get_cnt_tot(parm_handler_t->srch_key_file);

    return 0;
}