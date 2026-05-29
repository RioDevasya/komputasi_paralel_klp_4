// Copyright (C) 2026 Kelompok 4
#ifndef UTILITAS_H
#define UTILITAS_H

#include <stdio.h>

#if defined(_WIN32)
    #define OUTPUT_FOLDER "output\\"
#elif (defined(__APPLE__) && defined(__MACH__)) ||  defined(__linux__)
    #define OUTPUT_FOLDER "output/"
#else
    #define OUTPUT_FOLDER ""
#endif

#define CACHE_LINE_SIZE 64
#define MAX_LINES 21000
#define MAX_CHARACTERS 30
#define MAX_FILENAME_LENGTH 40
#define PRINT_FILENAME "output.txt"
#define PRINT_PATH OUTPUT_FOLDER PRINT_FILENAME

#define PROGRAM_EVENT_LOG_FILENAME "log_program.csv"
#define PROGRAM_EVENT_LOG_PATH OUTPUT_FOLDER PROGRAM_EVENT_LOG_FILENAME

// Threshold untuk menghindari overhead thread
#define THRESHOLD 300

// 1,2,4
#define TEST_COUNT 3
#define MAX_THREADS 4
#define MAX_PROCESS 4

// untuk peforma 64 bit
struct __attribute__((aligned(64))) ThreadStats {
    double merge_time;
    unsigned long merge_count;
    unsigned long task_created;
};

// I/O
void read_input(
    char const * const filename,
    char array[][MAX_CHARACTERS],
    int *size
);
void write_output(
    char const *filename, 
    char array[][MAX_CHARACTERS], 
    int size
);

// merge
void merge(char array[][MAX_CHARACTERS], int left, int mid, int right);

// Utilitas
int compare_strings(char const *a, char const *b);
void log_sort_event(
    int const id,
    int const task_created,
    int const merge_count, 
    double const merge_time,
    FILE *log_file_ptr
);
void log_program_event(
    int const rank_number,
    int const thread_number,
    int const array_size,
    char const * const event, 
    double const sort_time,
    double const program_time,
    FILE *log_file_ptr
);
void copy_array(
    char const source[][MAX_CHARACTERS],
    char destination[][MAX_CHARACTERS],
    int const array_size
);
int min(int const a, int const b);
int max(int const a, int const b);

#endif