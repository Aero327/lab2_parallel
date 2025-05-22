#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define N 500000
#define MAX_VAL 100

void sum(int *arr1, int *arr2);
void diff(int *arr1, int *arr2);
void mult(int *arr1, int *arr2);
void divv(int *arr1, int *arr2);

int main() {
    srand(time(NULL));
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
        { sum(array1, array2); }
        
        #pragma omp section
        { diff(array1, array2); }
        
        #pragma omp section
        { mult(array1, array2); }
        
        #pragma omp section
        { divv(array1, array2); }
    }
    end = omp_get_wtime();

    printf("Parallel time: %.5f seconds\n", end - start);

    free(array1);
    return 0;
}

void sum(int *arr1, int *arr2) {
    int *sum_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        sum_arr[i] = arr1[i] + arr2[i];
    }

    free(sum_arr);
}

void diff(int *arr1, int *arr2) {
    int *diff_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        diff_arr[i] = arr1[i] - arr2[i];
    }

    free(diff_arr);
}

void mult(int *arr1, int *arr2) {
    int *mult_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        mult_arr[i] = arr1[i] * arr2[i];
    }

    free(mult_arr);
}

void divv(int *arr1, int *arr2) {
    int *div_arr = (int*)malloc(N * sizeof(int));

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        div_arr[i] = arr1[i] / arr2[i];
    }

    free(div_arr);
}