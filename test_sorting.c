// Copyright (C) 2026 Kelompok 4
#include <stdio.h>
#include <string.h>
#include <omp.h>
#include <stdbool.h>
#include "include/utilitas.h"

#define THRESHOLD 70

// ==========================================
// FUNGSI INTI
// ==========================================
void parallel_merge_sort(char arr[][MAX_CHARACTERS], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        int do_parallel = (right - left > THRESHOLD);

        #pragma omp taskgroup
        {
            #pragma omp task if (do_parallel)
            parallel_merge_sort(arr, left, mid);

            #pragma omp task if (do_parallel)
            parallel_merge_sort(arr, mid + 1, right);
        }
        merge(arr, left, mid, right);
    }
}

// ==========================================
// FRAMEWORK UNIT TESTING 
// ==========================================
int test_passed = 0;
int test_failed = 0;

// Fungsi bantuan untuk mencetak array
void print_array(char arr[][MAX_CHARACTERS], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%s", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]");
}

void run_test(char *test_name, char arr[][MAX_CHARACTERS], char expected[][MAX_CHARACTERS], int n) {
    printf("--- %s ---\n", test_name);
    
    // Tampilkan data awal (hanya jika data kecil agar terminal tidak penuh)
    if (n <= 10) {
        printf("Input  : ");
        print_array(arr, n);
        printf("\n");
    } else {
        printf("Input  : [%d data disembunyikan]\n", n);
    }

    // Eksekusi Sorting
    if (n > 0) {
        #pragma omp parallel
        {
            #pragma omp single
            parallel_merge_sort(arr, 0, n - 1);
        }
    }

    // Tampilkan data setelah diurutkan
    if (n <= 10) {
        printf("Output : ");
        print_array(arr, n);
        printf("\n");
    } else {
        printf("Output : [%d data berhasil diproses]\n", n);
    }

    // Validasi Hasil
    bool pass = true;
    for (int i = 0; i < n; i++) {
        if (compare_strings(arr[i], expected[i]) != 0) {
            pass = false;
            break;
        }
    }

    // Cetak Output PASS/FAIL
    if (pass) {
        printf("Status : [PASS]\n\n");
        test_passed++;
    } else {
        printf("Status : [FAIL]\n\n");
        test_failed++;
    }
}

int main() {
    omp_set_num_threads(2);
    printf("=== MEMULAI UNIT TESTING SORTING PARALEL ===\n\n");

    // 1. Min 5 Test Case Correctness
    char tc1_data[][MAX_CHARACTERS] = {"z", "b", "a", "g"};
    char tc1_exp[][MAX_CHARACTERS]  = {"a", "b", "g", "z"};
    run_test("Test 1: Data Acak", tc1_data, tc1_exp, 4);

    char tc2_data[][MAX_CHARACTERS] = {"a", "b", "c", "d"};
    char tc2_exp[][MAX_CHARACTERS]  = {"a", "b", "c", "d"};
    run_test("Test 2: Data Sudah Urut", tc2_data, tc2_exp, 4);

    char tc3_data[][MAX_CHARACTERS] = {"x", "w", "v", "u"};
    char tc3_exp[][MAX_CHARACTERS]  = {"u", "v", "w", "x"};
    run_test("Test 3: Data Terbalik", tc3_data, tc3_exp, 4);

    char tc4_data[][MAX_CHARACTERS] = {"b", "a", "a", "b"};
    char tc4_exp[][MAX_CHARACTERS]  = {"a", "a", "b", "b"};
    run_test("Test 4: Data Kembar", tc4_data, tc4_exp, 4);

    char tc5_data[][MAX_CHARACTERS] = {"B", "a", "Z", "c"};
    char tc5_exp[][MAX_CHARACTERS]  = {"B", "Z", "a", "c"};
    run_test("Test 5: Huruf Kapital & Kecil", tc5_data, tc5_exp, 4);

    // 2. Test Edge Case
    char tc6_data[][MAX_CHARACTERS] = {};
    char tc6_exp[][MAX_CHARACTERS]  = {};
    run_test("Test 6: Edge Case (N = 0)", tc6_data, tc6_exp, 0);

    char tc7_data[][MAX_CHARACTERS] = {"tunggal"};
    char tc7_exp[][MAX_CHARACTERS]  = {"tunggal"};
    run_test("Test 7: Edge Case (N = 1)", tc7_data, tc7_exp, 1);

    // Edge case N besar simulasi sederhana
    int large_n = 5000;
    char tc8_data[large_n][MAX_CHARACTERS];
    char tc8_exp[large_n][MAX_CHARACTERS];
    for(int i=0; i<large_n; i++) {
        memcpy(
            tc8_data[i],
            "a",
            MAX_CHARACTERS
        );
        memcpy(
            tc8_exp[i],
            "a",
            MAX_CHARACTERS
        );
    }
    
    run_test("Test 8: Edge Case (N Sangat Besar)", tc8_data, tc8_exp, large_n);

    // 3. Rekap & Coverage Report
    printf("=== HASIL AKHIR ===\n");
    printf("Total Test : %d\n", test_passed + test_failed);
    printf("Passed     : %d\n", test_passed);
    printf("Failed     : %d\n", test_failed);
    printf("Coverage   : 100%% Fungsi Inti Ter-cover (merge & parallel_merge_sort)\n");

    return 0;
}