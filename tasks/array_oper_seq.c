#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#define MAX_VAL 100

void sum(int *sum_arr, int *arr1, int *arr2, int N);
void diff(int *diff_arr, int *arr1, int *arr2, int N);
void mult(int *mult_arr, int *arr1, int *arr2, int N);
void divv(int *div_arr, int *arr1, int *arr2, int N);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    int N = 100000;
    int opt;

    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                N = atoi(optarg);
                if (N <= 0) {
                    fprintf(stderr, "Error: N must be a positive integer.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-n array_size]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        array1[i] = rand() % MAX_VAL + 1;
        array2[i] = rand() % MAX_VAL + 1;
    }

    int *sum_arr = (int*)malloc(N * sizeof(int));
    int *diff_arr = (int*)malloc(N * sizeof(int));
    int *mult_arr = (int*)malloc(N * sizeof(int));
    int *div_arr = (int*)malloc(N * sizeof(int));

    double start, end;

    start = omp_get_wtime();
    sum(sum_arr, array1, array2, N);
    diff(diff_arr, array1, array2, N);
    mult(mult_arr, array1, array2, N);
    divv(div_arr, array1, array2, N);
    end = omp_get_wtime();

    printf("Sequential time: %.5f seconds\n", end - start);

    free(array1);
    free(array2);
    return 0;
}

void sum(int *sum_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        sum_arr[i] = arr1[i] + arr2[i];
    }

    free(sum_arr);
}

void diff(int *diff_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        diff_arr[i] = arr1[i] - arr2[i];
    }

    free(diff_arr);
}

void mult(int *mult_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        mult_arr[i] = arr1[i] * arr2[i];
    }

    free(mult_arr);
}

void divv(int *div_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        div_arr[i] = arr1[i] / arr2[i];
    }

    free(div_arr);
}