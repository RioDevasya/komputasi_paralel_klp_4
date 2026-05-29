// Copyright (C) 2026 Kelompok 4
#include "omp.h"
#include "include/distributed.h"

void init_distributed_parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right
);

void distributed_parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right
);

void log_sort_event_paralel_distributed(
    int const rank_number,
    int const thread_id,
    int const task_created,
    int const merge_count, 
    double const merge_time,
    FILE *log_file_ptr
);



int main() {
    omp_set_num_threads(2);

    printf("Total thread: %d\n", omp_get_max_threads()),
    distributed_main_procedure(
        "STARTING DISTRIBUTED + PARALLEL MERGE SORT",
        "rank,thread_id,task_created,merge_count,merge_time\n",
        "dis_par",
        log_sort_event_paralel_distributed,
        init_distributed_parallel_merge_sort
    );

    return 0;
}

void init_distributed_parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right
) {
    #pragma omp parallel
    {
        #pragma omp single
        distributed_parallel_merge_sort(
            array,
            left,
            right
        );
    }
}

void distributed_parallel_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right
) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        int do_parallel = (right - left > THRESHOLD);

        #pragma omp taskgroup
        {
            #pragma omp task if (do_parallel)
            distributed_parallel_merge_sort(
                array, left, mid
            );

            #pragma omp task if (do_parallel)
            distributed_parallel_merge_sort(
                array, mid + 1, right
            );
        }

        int thread_id = omp_get_thread_num();

        double merge_start = omp_get_wtime();
        merge(array, left, mid, right);

        threadStats[mpi_rank][thread_id].merge_time += omp_get_wtime() - merge_start;
        threadStats[mpi_rank][thread_id].merge_count++;
        if (do_parallel)
            threadStats[mpi_rank][thread_id].task_created += 2;
    }
}

void log_sort_event_paralel_distributed(
    int const rank_number,
    int const thread_id,
    int const task_created,
    int const merge_count, 
    double const merge_time,
    FILE *log_file_ptr
) {
    fprintf(log_file_ptr, "%d,%d,%d,%d,%lf\n",
        rank_number,
        thread_id,
        task_created,
        merge_count, 
        merge_time
    );

    fflush(log_file_ptr);
}