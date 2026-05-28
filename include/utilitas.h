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

#define MAX_LINES 3000
#define MAX_CHARACTERS 30
#define MAX_FILENAME_LENGTH 40
#define PRINT_FILENAME "output.txt"
#define PRINT_PATH OUTPUT_FOLDER PRINT_FILENAME

// Threshold untuk menghindari overhead thread
#define THRESHOLD 70

// 1,2,4
#define TEST_COUNT 3

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
void print_array(char *array[], int size);
int compare_strings(char const *a, char const *b);
void log_event(
    int const id,
    char const *event, 
    int const left, 
    int const mid, 
    int const right, 
    int const depth, 
    double const time,
    FILE* log_file_ptr
);
void copy_array(
    char const source[][MAX_CHARACTERS],
    char destination[][MAX_CHARACTERS],
    int const array_size
);

#endif