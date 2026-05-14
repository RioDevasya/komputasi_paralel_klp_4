#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

// Full Execution Flow
// 1. parallel → create thread pool
// 2. single → one thread starts recursion
// 3. recursion creates tasks
// 4. threads steal tasks and execute them
// 5. taskwait ensures correctness
// 6. merge happens bottom-up

// Threshold untuk menghindari overhead thread
#define THRESHOLD 70
#define MAX_LINE 3000

FILE *log_fp;

// I/O
char** read_input(const char* filename, int* size);
void write_output(const char* filename, char** data, int size);

// merge
void merge(char *arr[], int left, int mid, int right);

// sorting
void parallel_merge_sort(char *arr[], int left, int right, int depth);

// Utilitas
void print_array(char *arr[], int size);
int compare_strings(const char* a, const char* b);
void log_event(const char *event, int left, int mid, int right, int depth);

int main() {
    omp_set_num_threads(2);

    log_fp = fopen("log.csv", "w");
    if (!log_fp) {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    // header CSV
    fprintf(log_fp, "thread_id,event,left,mid,right,depth\n");

    int n = 0;
    char **arr = read_input("input.txt", &n);

    if (!arr) {
        return 1;
    }
    printf("Total threads %d\n", omp_get_max_threads());
    printf("Read %d lines\n", n);

    double start = omp_get_wtime();

    // Paralelisasi
    #pragma omp parallel
    {
        #pragma omp single
        parallel_merge_sort(arr, 0, n - 1, 0);
    }

    double end = omp_get_wtime();

    write_output("output.txt", arr, n);

    printf("Sorted output written to output.txt\n");
    printf("Time taken: %lf seconds\n", end - start);

    // free memory
    for (int i = 0; i < n; i++) {
        free(arr[i]);
    }
    free(arr);

    fclose(log_fp);
    return 0;
}



char** read_input(const char* filename, int* size) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
    }

    char **arr = NULL;
    int capacity = 0;
    int n = 0;
    char buffer[MAX_LINE];

    while (fgets(buffer, MAX_LINE, fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        if (n >= capacity) {
            capacity = capacity == 0 ? 16 : capacity * 2;
            char **temp = realloc(arr, capacity * sizeof(char *));
            if (!temp) {
                perror("Memory allocation failed");
                fclose(fp);
                exit(EXIT_FAILURE);
            }
            arr = temp;
        }

        arr[n++] = strdup(buffer);
    }

    fclose(fp);

    *size = n;
    return arr;
}

void write_output(const char* filename, char** data, int size) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        fprintf(fp, "%s\n", data[i]);
    }

    fclose(fp);
}



void merge(char *arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    char **L = malloc(n1 * sizeof(char *));
    if (!L) {
        perror("Failed to malloc L");
        exit(EXIT_FAILURE);
    }

    char **R = malloc(n2 * sizeof(char *));
    if (!R) {
        free(L);
        perror("Failed to malloc R");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0; j = 0; k = left;

    while (i < n1 && j < n2) {
        if (compare_strings(L[i], R[j]) <= 0)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1)
        arr[k++] = L[i++];

    while (j < n2)
        arr[k++] = R[j++];

    free(L);
    free(R);
}



void parallel_merge_sort(char *arr[], int left, int right, int depth) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        int do_parallel = (right - left > THRESHOLD);

        log_event("divide", left, mid, right, depth);

        #pragma omp taskgroup
        {
            #pragma omp task if (do_parallel)
            parallel_merge_sort(arr, left, mid, depth + 1);

            #pragma omp task if (do_parallel)
            parallel_merge_sort(arr, mid + 1, right, depth + 1);
        }

        log_event("merge", left, mid, right, depth);

        merge(arr, left, mid, right);
    }
}



void print_array(char *arr[], int size) {
    for (int i = 0; i < size; i++)
        printf("%s\n", arr[i]);
}

int compare_strings(const char* a, const char* b) {
    return strcmp(a, b);
    // return -strcmp(a, b); descending
    // return strcasecmp(a, b); case-insensitive
    // return strlen(a) - strlen(b); sort-by-length
}

void log_event(const char *event, int left, int mid, int right, int depth) {
    int tid = omp_get_thread_num();

    fprintf(log_fp, "%d,%s,%d,%d,%d,%d\n",
            tid, event, left, mid, right, depth);
    fflush(log_fp);
}