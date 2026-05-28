// Copyright (C) 2026 Kelompok 4
#include <stdlib.h>
#include <mpi.h>
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




void distributed_main_procedure(
    char *sort_message,
    char *log_column_header,
    char *method_name_for_filename,
    void (*log_rank_0_first_divide_last_merge)(
        int const rank,
        char const *event, 
        int const left, 
        int const mid, 
        int const right, 
        int const depth, 
        double const time,
        FILE* log_file_ptr
    ),
    void (*merge_sort_function)(
        char array[MAX_LINES][MAX_CHARACTERS], 
        int const left, 
        int const right, 
        int const depth, 
        FILE *log_file_ptr
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

    fprintf(
        log_file_ptr,
        "%s",
        log_column_header
    );

    if (mpi_rank == 0) {
        snprintf(
            output_filename, 
            sizeof(output_filename), 
            "%soutput_%s_%d_process.txt",
            OUTPUT_FOLDER,
            method_name_for_filename,
            mpi_process_count
        );

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
    double start = MPI_Wtime();

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

    if (mpi_rank == 0)
        log_rank_0_first_divide_last_merge(
            mpi_rank,
            "divide_to_workers",
            0,
            array_size / 2,
            array_size,
            0,
            MPI_Wtime() - start,
            log_file_ptr
        );

    // depth 1 because we already divide once
    merge_sort_function(
        local_array,
        0,
        local_size-1,
        1,
        log_file_ptr
    );

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

        double end = MPI_Wtime();

        log_rank_0_first_divide_last_merge(
            mpi_rank,
            "merge_worker_chunks",
            0,
            array_size / 2,
            array_size,
            0,
            end - start_last_merge,
            log_file_ptr
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
            "Time taken: %.6lf seconds\n\n",
            end-start
        );

        fclose(log_file_ptr);
    }

    free(local_array);

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
    int const right, 
    int const depth, 
    FILE *log_file_ptr
) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        double divide_start = MPI_Wtime();

        distributed_merge_sort(
            array, left, mid, depth + 1, log_file_ptr
        );

        distributed_merge_sort(
            array, mid + 1, right, depth + 1, log_file_ptr
        );

        double divide_end = MPI_Wtime();

        log_event(
            mpi_rank,
            "divide", 
            left, 
            mid, 
            right, 
            depth, 
            divide_end - divide_start,
            log_file_ptr
        );

        double merge_start = MPI_Wtime();
        merge(array, left, mid, right);
        double merge_end = MPI_Wtime();

        log_event(
            mpi_rank,
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