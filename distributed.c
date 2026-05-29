// Copyright (C) 2026 Kelompok 4
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include "include/utilitas.h"
#include "include/distributed.h"

// variable
char array[MAX_LINES][MAX_CHARACTERS];
int mpi_rank;
int mpi_process_count;
char output_filename[MAX_FILENAME_LENGTH], 
    log_filename[MAX_FILENAME_LENGTH];
int array_size = 0;
FILE *log_file_ptr = NULL;
FILE *log_program_event_ptr = NULL;
struct ThreadStats threadStats[MAX_PROCESS][MAX_THREADS];



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
) {
    MPI_Init(NULL, NULL);

    MPI_Comm_rank(
        MPI_COMM_WORLD,
        &mpi_rank
    );

    MPI_Comm_size(
        MPI_COMM_WORLD,
        &mpi_process_count
    );

    double end_init = MPI_Wtime();

    snprintf(
        log_filename, 
        sizeof(log_filename), 
        "%slog_%s_%dprc_rank_%d.csv",
        OUTPUT_FOLDER,
        method_name_for_filename,
        mpi_process_count,
        mpi_rank
    );

    log_file_ptr = fopen(
        log_filename,
        "w"
    );

    if (!log_file_ptr) {

        perror("Failed to open log");

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );
    }

    if (mpi_rank == 0) {
        snprintf(
            output_filename, 
            sizeof(output_filename), 
            "%soutput_%s_%d_process.txt",
            OUTPUT_FOLDER,
            method_name_for_filename,
            mpi_process_count
        );

        log_program_event_ptr = fopen(
            PROGRAM_EVENT_LOG_PATH,
            "a"
        );

        if (!log_program_event_ptr) {
            perror("Failed to open log file");
            MPI_Abort(
                MPI_COMM_WORLD,
                EXIT_FAILURE
            );
        }

        read_input("input.txt", array, &array_size);
        printf(
            "Total process %d\n"
            "Read %d lines from input.txt\n"
            "%s\n", 
            mpi_process_count,
            array_size,
            sort_message
        );
    }

    double start_data_count = MPI_Wtime();

    MPI_Bcast(
        &array_size,
        1,
        MPI_INT,
        0,
        MPI_COMM_WORLD
    );

    // Hitung pembagian data
    int send_counts[mpi_process_count];
    int displacements[mpi_process_count];

    int base = array_size / mpi_process_count;
    int remainder = array_size % mpi_process_count;

    int offset = 0;

    for (int i = 0; i < mpi_process_count; i++) {
        int current_size = base + (i < remainder);
        send_counts[i] = current_size * MAX_CHARACTERS;
        displacements[i] = offset;
        offset += send_counts[i];
    }

    int local_size = send_counts[mpi_rank] / MAX_CHARACTERS;

    char (*local_array)[MAX_CHARACTERS] = malloc(
        local_size*
        sizeof(*local_array)
    );

    if (!local_array) {

        perror("malloc local_array");

        MPI_Abort(
            MPI_COMM_WORLD,
            EXIT_FAILURE
        );
    }

    double start_merge_sort = MPI_Wtime();

    MPI_Scatterv(
        array,
        send_counts,
        displacements,
        MPI_CHAR,

        local_array,
        send_counts[mpi_rank],
        MPI_CHAR,

        0,
        MPI_COMM_WORLD
    );


    merge_sort_function(
        local_array,
        0,
        local_size-1
    );

    double end_merge_sort = MPI_Wtime();

    MPI_Gatherv(
        local_array,
        send_counts[mpi_rank],
        MPI_CHAR,

        array,
        send_counts,
        displacements,
        MPI_CHAR,

        0,
        MPI_COMM_WORLD
    );

    // Merge final
    if (mpi_rank == 0) {
        double start_last_merge = MPI_Wtime();

        merge_worker_chunks(
            array,
            send_counts
        );

        double end_program = MPI_Wtime();
        double program_time = end_program - start_data_count + end_init;
        double sort_time = (end_program - start_last_merge) + (end_merge_sort - start_merge_sort);

        threadStats[mpi_rank][0].merge_time += end_program - start_last_merge;
        threadStats[mpi_rank][0].merge_count++;

        char program_name[MAX_FILENAME_LENGTH];

        snprintf(
            program_name,
            MAX_FILENAME_LENGTH,
            "Processs %d - Threads %d",
            mpi_process_count,
            omp_get_max_threads()
        );

        log_program_event(
            mpi_process_count,
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
    }

    fprintf(
        log_file_ptr,
        "%s",
        log_column_header
    );
    
    for (int i = 0; i < max(omp_get_max_threads(), 1); i++)
        log_sort_event(
            mpi_rank,
            i,
            threadStats[mpi_rank][i].task_created,
            threadStats[mpi_rank][i].merge_count,
            threadStats[mpi_rank][i].merge_time,
            log_file_ptr
        );

    free(local_array);

    fclose(log_file_ptr);

    MPI_Finalize();
}



void merge_worker_chunks(
    char array[][MAX_CHARACTERS],
    int const send_counts[]
) {

    // Hitung ukuran chunk
    int chunk_sizes[mpi_process_count];

    for (int i = 0; i < mpi_process_count; i++) {
        chunk_sizes[i] = send_counts[i] / MAX_CHARACTERS;
    }

    // Hitung posisi awal tiap chunk
    int offsets[mpi_process_count];
    offsets[0] = 0;

    for (int i = 1; i < mpi_process_count; i++) {
        offsets[i] = offsets[i - 1] + chunk_sizes[i - 1];
    }

    // Merge bertingkat:
    // 0+1
    // 2+3
    // hasil+hasil
    int active_chunks = mpi_process_count;

    while (active_chunks > 1) {
        int next = 0;

        for (int i = 0; i < active_chunks; i += 2) {
            if (i + 1 >= active_chunks) {
                offsets[next] = offsets[i];
                chunk_sizes[next] = chunk_sizes[i];
                next++;
                continue;
            }

            int left_start = offsets[i];
            int left_size = chunk_sizes[i];

            int right_start = offsets[i + 1];
            int right_size = chunk_sizes[i + 1];

            int total = left_size + right_size;

            char (*temp)[MAX_CHARACTERS] =
                malloc(total * sizeof(*temp));

            if (!temp) {
                perror("malloc temp");
                exit(EXIT_FAILURE);
            }

            int l = 0;
            int r = 0;
            int t = 0;

            while (l < left_size && r < right_size) {
                if (
                    compare_strings(
                        array[left_start + l],
                        array[right_start + r]
                    ) <= 0
                ) {
                    memcpy(
                        temp[t++],
                        array[left_start + l++],
                        sizeof(temp[0])
                    );
                }

                else {
                    memcpy(
                        temp[t++],
                        array[right_start + r++],
                        sizeof(temp[0])
                    );
                }
            }

            while (l < left_size) {
                memcpy(
                    temp[t++],
                    array[left_start + l++],
                    sizeof(temp[0])
                );
            }

            while (r < right_size) {
                memcpy(
                    temp[t++],
                    array[right_start + r++],
                    sizeof(temp[0])
                );
            }

            memcpy(
                &array[left_start],
                temp,
                total * sizeof(*temp)
            );

            free(temp);

            offsets[next] = left_start;
            chunk_sizes[next] = total;
            next++;
        }

        active_chunks = next;
    }
}

void distributed_merge_sort(
    char array[MAX_LINES][MAX_CHARACTERS], 
    int const left, 
    int const right
) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        distributed_merge_sort(
            array, left, mid
        );

        distributed_merge_sort(
            array, mid + 1, right
        );
        
        double merge_start = MPI_Wtime();
        merge(array, left, mid, right);

        threadStats[mpi_rank][0].merge_time += MPI_Wtime() - merge_start;
        threadStats[mpi_rank][0].merge_count++;
    }
}

void log_distributed_sort_event(
    int const rank_number,
    int const thread_id,
    int const task_created,
    int const merge_count, 
    double const merge_time,
    FILE *log_file_ptr
) {
    log_sort_event(
        rank_number,
        task_created,
        merge_count,
        merge_time,
        log_file_ptr
    );
}