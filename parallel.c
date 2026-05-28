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

// sorting
void parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right, 
    int const depth, 
    FILE *log_file_ptr
);

void parallel_main_function(
    int const thread_number,
    char array[][MAX_CHARACTERS],
    int const array_size,
    char const * const output_filename,
    char const * const log_filename
);

void process_filename(char filename[MAX_FILENAME_LENGTH]);



void parallel_main_function(
    int const thread_number,
    char array[][MAX_CHARACTERS],
    int const array_size,
    char const * const output_filename,
    char const * const log_filename
) {
    omp_set_num_threads(thread_number);
    printf("Total threads %d\n", omp_get_max_threads());

    FILE *log_file_ptr = fopen(log_filename, "w");

    if (!log_file_ptr) {
        perror("Failed to open log file");

        exit(EXIT_FAILURE);
    }

    fprintf(
        log_file_ptr,
        "thread_id,event,left,mid,right,depth,time\n"
    );

    printf(
        "STARTING PARALLEL MERGE SORT --- Total threads: %d\n", 
        thread_number
    );

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        parallel_merge_sort(
            array,
            0,
            array_size - 1,
            0,
            log_file_ptr
        );
    }

    double end = omp_get_wtime();

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
        "Time taken: %.6lf seconds\n\n",
        end - start
    );

    fclose(log_file_ptr);
}




void parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right, 
    int const depth, 
    FILE *log_file_ptr
) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        int do_parallel = (right - left > THRESHOLD);
        int thread_id = omp_get_thread_num();

        double divide_start = omp_get_wtime();

        #pragma omp taskgroup
        {
            #pragma omp task if (do_parallel)
            parallel_merge_sort(
                array, left, mid, depth + 1, log_file_ptr
            );

            #pragma omp task if (do_parallel)
            parallel_merge_sort(
                array, mid + 1, right, depth + 1, log_file_ptr
            );
        }

        double divide_end = omp_get_wtime();

        log_event(
            thread_id,
            "divide", 
            left, 
            mid, 
            right, 
            depth, 
            divide_end - divide_start,
            log_file_ptr
        );

        double merge_start = omp_get_wtime();
        merge(array, left, mid, right);
        double merge_end = omp_get_wtime();

        log_event(
            thread_id,
            "merge", 
            left, 
            mid, 
            right, 
            depth, 
            merge_end - merge_start,
            log_file_ptr
        );
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