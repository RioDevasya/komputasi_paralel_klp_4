// Copyright (C) 2026 Kelompok 4
#include "omp.h"
#include "include/distributed.h"

void init_distributed_parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right, 
    int const depth, 
    FILE *log_file_ptr
);

void distributed_parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right, 
    int const depth, 
    FILE *log_file_ptr
);

void log_rank_0_first_divide_last_merge(
    int const rank,
    char const * const event, 
    int const left, 
    int const mid, 
    int const right, 
    int const depth, 
    double const time,
    FILE *log_file_ptr
);

void log_distributed_and_parallel_event(
    int const rank,
    int const thread_id,
    char const * const event, 
    int const left, 
    int const mid, 
    int const right, 
    int const depth, 
    double const time,
    FILE *log_file_ptr
);



int main() {
    omp_set_num_threads(2);

    printf("Total thread: %d\n", omp_get_max_threads()),
    distributed_main_procedure(
        "STARTING DISTRIBUTED + PARALLEL MERGE SORT",
        "rank,thread_id,event,left,mid,right,depth,time\n",
        "dis_par",
        log_rank_0_first_divide_last_merge,
        init_distributed_parallel_merge_sort
    );

    return 0;
}

void init_distributed_parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right, 
    int const depth, 
    FILE *log_file_ptr
) {
    #pragma omp parallel
    {
        #pragma omp single
        distributed_parallel_merge_sort(
            array,
            left,
            right,
            depth,
            log_file_ptr
        );
    }
}

void distributed_parallel_merge_sort(
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
            distributed_parallel_merge_sort(
                array, left, mid, depth + 1, log_file_ptr
            );

            #pragma omp task if (do_parallel)
            distributed_parallel_merge_sort(
                array, mid + 1, right, depth + 1, log_file_ptr
            );
        }

        double divide_end = omp_get_wtime();

        log_distributed_and_parallel_event(
            mpi_rank,
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

        log_distributed_and_parallel_event(
            mpi_rank,
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

void log_rank_0_first_divide_last_merge(
    int const rank,
    char const * const event, 
    int const left, 
    int const mid, 
    int const right, 
    int const depth, 
    double const time,
    FILE *log_file_ptr
) {
    fprintf(log_file_ptr, "%d,%d,%s,%d,%d,%d,%d,%lf\n",
        rank,
        omp_get_thread_num(), 
        event, 
        left, 
        mid, 
        right, 
        depth, 
        time
    );

    fflush(log_file_ptr);
}

void log_distributed_and_parallel_event(
    int const rank,
    int const thread_id,
    char const * const event, 
    int const left, 
    int const mid, 
    int const right, 
    int const depth, 
    double const time,
    FILE *log_file_ptr
) {
    fprintf(log_file_ptr, "%d,%d,%s,%d,%d,%d,%d,%lf\n",
        rank,
        thread_id, 
        event, 
        left, 
        mid, 
        right, 
        depth, 
        time
    );

    fflush(log_file_ptr);
}