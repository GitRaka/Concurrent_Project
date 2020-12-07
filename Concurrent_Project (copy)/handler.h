// Author: Rakesh Kumar

struct parm_handler{
    char *in_file;
    char *srch_key_file;
    char *rng_file;
    char my_name[16];
    char *lock;
    int num_ins_keys;
    int num_srch_keys;
    int rng_queries;
    int tot_thrds;
    bool lock_rw_set;
};

int parser(int argc, char **argv, struct parm_handler *parm_handler_t);

// int count_parser(int argc, char **argv, struct );

//int array_insert(struct parm_handler parm_handler_t, int list[]);

//int file_insert();

//void console_print(struct parm_handler parm_handler_t, int list[]);