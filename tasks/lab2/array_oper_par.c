#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#define MAX_VAL 100

void sum(int *arr1, int *arr2, int N);
void diff(int *arr1, int *arr2, int N);
void mult(int *arr1, int *arr2, int N);
void divv(int *arr1, int *arr2, int N);

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

    double start, end;

    start = omp_get_wtime();
    #pragma omp parallel sections
    {
        #pragma omp section
        { sum(array1, array2, N); }
        
        #pragma omp section
        { diff(array1, array2, N); }
        
        #pragma omp section
        { mult(array1, array2, N); }
        
        #pragma omp section
        { divv(array1, array2, N); }
    }
    end = omp_get_wtime();

    printf("Parallel time: %.5f seconds\n", end - start);

    free(array1);
    free(array2);
    return 0;
}

void sum(int *arr1, int *arr2, int N) {
    int *sum_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        sum_arr[i] = arr1[i] + arr2[i];
    }
}

void diff(int *arr1, int *arr2, int N) {
    int *diff_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        diff_arr[i] = arr1[i] - arr2[i];
    }
}

void mult(int *arr1, int *arr2, int N) {
    int *mult_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        mult_arr[i] = arr1[i] * arr2[i];
    }
}

void divv(int *arr1, int *arr2, int N) {
    int *div_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        div_arr[i] = arr1[i] / arr2[i];
    }
}