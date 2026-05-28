// Copyright (C) 2026 Kelompok 4
#include <stdlib.h>
#include <omp.h>
#include <mpi.h>
#include <string.h>
#include "include/utilitas.h"

void read_input(
    char const * const filename,
    char array[][MAX_CHARACTERS],
    int *size
) {
    FILE *file_ptr = fopen(filename, "r");

    if (!file_ptr) {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    int lines = 0;

    while (
        lines < MAX_LINES &&
        fgets(array[lines], MAX_CHARACTERS, file_ptr)
    ) {

        array[lines][
            strcspn(
                array[lines],
                "\n"
            )
        ] = '\0';

        lines++;
    }

    fclose(file_ptr);

    *size = lines;
}

void write_output(char const *filename, char array[][MAX_CHARACTERS], int size) {
    FILE *file_ptr = fopen(filename, "w");
    if (!file_ptr) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        fprintf(file_ptr, "%s\n", array[i]);
    }

    fclose(file_ptr);
}

void merge(
    char array[][MAX_CHARACTERS],
    int const left,
    int const mid,
    int const right
) {
    int i, j, k;

    int const n1 = mid - left + 1;
    int const n2 = right - mid;

    char (*L)[MAX_CHARACTERS] =
        malloc(n1 * sizeof(*L));
    if (!L) {
        perror("Failed to malloc L");
        exit(EXIT_FAILURE);
    }

    char (*R)[MAX_CHARACTERS] =
        malloc(n2 * sizeof(*R));
    if (!R) {
        free(L);
        perror("Failed to malloc R");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < n1; i++) {
        memcpy(
            L[i],
            array[left + i],
            MAX_CHARACTERS
        );
    }

    for (j = 0; j < n2; j++) {
        memcpy(
            R[j],
            array[mid + 1 + j],
            MAX_CHARACTERS
        );
    }

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2) {

        if (
            compare_strings(
                L[i],
                R[j]
            ) <= 0
        ) {

            memcpy(
                array[k++],
                L[i++],
                MAX_CHARACTERS
            );

        } else {

            memcpy(
                array[k++],
                R[j++],
                MAX_CHARACTERS
            );
        }
    }

    while (i < n1) {

        memcpy(
            array[k++],
            L[i++],
            MAX_CHARACTERS
        );
    }

    while (j < n2) {

        memcpy(
            array[k++],
            R[j++],
            MAX_CHARACTERS
        );
    }

    free(L);
    free(R);
}


void print_array(char *array[], int size) {
    for (int i = 0; i < size; i++)
        printf("%s\n", array[i]);
}

int compare_strings(char const* a, char const* b) {
    return strcmp(a, b);
    // return -strcmp(a, b); descending
    // return strcasecmp(a, b); case-insensitive
    // return strlen(a) - strlen(b); sort-by-length
}

void log_event(
    int const id,
    char const * const event, 
    int const left, 
    int const mid, 
    int const right, 
    int const depth, 
    double const time,
    FILE *log_file_ptr
) {
    fprintf(log_file_ptr, "%d,%s,%d,%d,%d,%d,%lf\n",
        id, 
        event, 
        left, 
        mid, 
        right, 
        depth, 
        time
    );

    fflush(log_file_ptr);
}

void copy_array(
    char const source[][MAX_CHARACTERS],
    char destination[][MAX_CHARACTERS],
    int const array_size
) {

    memcpy(
        destination,
        source,
        array_size * MAX_CHARACTERS
    );
}