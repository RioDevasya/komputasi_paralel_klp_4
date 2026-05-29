// Copyright (C) 2026 Kelompok 4
#ifndef DISTRIBUTED_H
#define DISTRIBUTED_H

#include "utilitas.h"
#include <stdio.h>

extern int mpi_rank;
extern int mpi_process_count;
extern struct ThreadStats threadStats[MAX_PROCESS][MAX_THREADS];

void distributed_main_procedure(
    char *sort_message,
    char *log_column_header,
    char *method_name_for_filename,
    void log_sort_event(
        int const rank_number,
        int const thread_id,
        int const task_created,
        int const merge_count, 
        double const merge_time,
        FILE *log_file_ptr
    ),
    void (*merge_sort_function)(
        char array[MAX_LINES][MAX_CHARACTERS], 
        int const left, 
        int const right
    )
);

void distributed_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right
);

void merge_worker_chunks(
    char array[][MAX_CHARACTERS],
    int const send_counts[]
);

#endif