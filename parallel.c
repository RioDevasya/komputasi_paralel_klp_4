// Copyright (C) 2026 Kelompok 4
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include "include/utilitas.h"

// Full Execution Flow
// 1. parallel → create thread pool
// 2. single → one thread starts recursion
// 3. recursion creates tasks
// 4. threads steal tasks and execute them
// 5. taskwait ensures correctness
// 6. merge happens bottom-up

struct Data {
    char input_array[MAX_LINES][MAX_CHARACTERS];

    char parallel_arrays
        [TEST_COUNT]
        [MAX_LINES]
        [MAX_CHARACTERS];
};
struct Data data;

struct ThreadStats threadStats[MAX_THREADS];

// sorting
void parallel_merge_sort(
    char array[][MAX_CHARACTERS], 
    int const left, 
    int const right
);

void parallel_main_function(
    int const is_first_log,
    int const thread_number,
    char array[][MAX_CHARACTERS],
    int const array_size,
    char const * const output_filename,
    char const * const log_filename
);

void process_filename(char filename[MAX_FILENAME_LENGTH]);



void parallel_main_function(
    int const is_first_log,
    int const thread_number,
    char array[][MAX_CHARACTERS],
    int const array_size,
    char const * const output_filename,
    char const * const log_filename
) {
    // init

    FILE *log_program_event_ptr = fopen(
        PROGRAM_EVENT_LOG_PATH,
        is_first_log ? "w" : "a"
    );
    FILE *log_file_ptr = fopen(log_filename, "w");

    if (!log_file_ptr || !log_program_event_ptr) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    printf("STARTING PARALLEL MERGE SORT\n");

    // sort
    double start_program = omp_get_wtime();

    omp_set_num_threads(thread_number);

    double start_sort = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        parallel_merge_sort(
            array,
            0,
            array_size - 1
        );
    }

    double end_program = omp_get_wtime();

    // log

    printf("Total threads %d\n", omp_get_max_threads());
    fprintf(
        log_file_ptr,
        "thread_id,task_created,merge_count,merge_time\n"
    );

    for (int i = 0; i < thread_number; i++) {
        log_sort_event(
            i,
            threadStats[i].task_created,
            threadStats[i].merge_count,
            threadStats[i].merge_time,
            log_file_ptr
        );
    }
    char program_name[MAX_FILENAME_LENGTH];
    double program_time = end_program - start_program;
    double sort_time = end_program - start_sort;

    snprintf(
        program_name,
        MAX_FILENAME_LENGTH,
        "Threads %d",
        omp_get_max_threads()
    );

    if (is_first_log)
        fprintf(
            log_program_event_ptr,
            "rank_number,thread_number,array_size,name,sort_time,program_time\n"
        );

    log_program_event(
        0,
        omp_get_max_threads(),
        array_size,
        program_name,
        sort_time,
        program_time,
        log_program_event_ptr
    );

    write_output(
        output_filename,
        array,
        array_size
    );

    printf(
        "Sorted output written to %s\n",
        output_filename
    );

    printf(
        "Program time taken: %.6lf seconds\n"
        "Sort time taken: %.6lf seconds\n\n",
        program_time,
        sort_time
    );

    fclose(log_file_ptr);
}



void parallel_merge_sort(
    char array[][MAX_CHARACTERS], 
    int const left, 
    int const right
) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        int do_parallel = (right - left > THRESHOLD);

        #pragma omp taskgroup
        {
            #pragma omp task if (do_parallel)
            parallel_merge_sort(
                array, left, mid
            );

            #pragma omp task if (do_parallel)
            parallel_merge_sort(
                array, mid + 1, right
            );
        }

        int thread_id = omp_get_thread_num();
        
        double merge_start = omp_get_wtime();
        merge(array, left, mid, right);

        threadStats[thread_id].merge_time += omp_get_wtime() - merge_start;
        threadStats[thread_id].merge_count++;
        if (do_parallel)
            threadStats[thread_id].task_created += 2;
    }
}



void give_filename_and_output_folder(
    char variable_output[MAX_FILENAME_LENGTH],
    char *filename
) {
    snprintf(
        variable_output,
        MAX_FILENAME_LENGTH,
        "%s%s",
        OUTPUT_FOLDER,
        filename
    );
}