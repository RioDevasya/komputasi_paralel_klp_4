#ifndef DISTRIBUTED_H
#define DISTRIBUTED_H

#include "utilitas.h"
#include <stdio.h>

extern int mpi_rank;

void distributed_main_procedure(
    char *sort_message,
    char *log_column_header,
    char *method_name_for_filename,
    void (*merge_sort_function)(
        char array[MAX_LINES][MAX_CHARACTERS], 
        int const left, 
        int const right, 
        int const depth, 
        FILE *log_file_ptr
    )
);

void distributed_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right, 
    int const depth, 
    FILE *log_file_ptr
);

void merge_worker_chunks(
    char array[][MAX_CHARACTERS],
    int const send_counts[]
);

#endif