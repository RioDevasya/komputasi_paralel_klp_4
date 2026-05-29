// Copyright (C) 2026 Kelompok 4
#include "parallel.c"

int main() {
    char output_filename[MAX_FILENAME_LENGTH], log_filename[MAX_FILENAME_LENGTH];
    int array_size = 0;
    read_input("input.txt", data.input_array, &array_size);
    printf("Read %d lines from input.txt\n", array_size);

    for (int i = 0; i < TEST_COUNT; i++) {
        copy_array(
            data.input_array,
            data.parallel_arrays[i],
            array_size
        );
    }

    // 1 thread
    give_filename_and_output_folder(
        output_filename,
        "output_parallel_1_thread.txt"
    );

    give_filename_and_output_folder(
        log_filename,
        "log_parallel_1_thread.csv"
    );

    parallel_main_function(
        1,
        1,
        data.parallel_arrays[0],
        array_size,
        output_filename,
        log_filename
    );

    // 2 thread
    give_filename_and_output_folder(
        output_filename,
        "output_parallel_2_thread.txt"
    );

    give_filename_and_output_folder(
        log_filename,
        "log_parallel_2_thread.csv"
    );
    
    parallel_main_function(
        0,
        2,
        data.parallel_arrays[1],
        array_size,
        output_filename,
        log_filename
    );

    // 4 thread
    give_filename_and_output_folder(
        output_filename,
        "output_parallel_4_thread.txt"
    );

    give_filename_and_output_folder(
        log_filename,
        "log_parallel_4_thread.csv"
    );

    parallel_main_function(
        0,
        4,
        data.parallel_arrays[2],
        array_size,
        output_filename,
        log_filename
    );

    return 0;
}