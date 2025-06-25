#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>  // Добавляем заголовок MPI

#define MAX_VAL 100

void sum(int *sum_arr, int *arr1, int *arr2, int N);
void diff(int *diff_arr, int *arr1, int *arr2, int N);
void mult(int *mult_arr, int *arr1, int *arr2, int N);
void divv(int *div_arr, int *arr1, int *arr2, int N);

int main(int argc, char* argv[]) {
    // Инициализация MPI (даже для последовательного выполнения)
    MPI_Init(&argc, &argv);
    
    srand(time(NULL));

    int N = 100000;
    int opt;

    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                N = atoi(optarg);
                if (N <= 0) {
                    fprintf(stderr, "Error: N must be a positive integer.\n");
                    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-n array_size]\n", argv[0]);
                MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    
    if (array1 == NULL || array2 == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < N; i++) {
        array1[i] = rand() % MAX_VAL + 1;
        array2[i] = rand() % MAX_VAL + 1;
    }

    int *sum_arr = (int*)malloc(N * sizeof(int));
    int *diff_arr = (int*)malloc(N * sizeof(int));
    int *mult_arr = (int*)malloc(N * sizeof(int));
    int *div_arr = (int*)malloc(N * sizeof(int));
    
    if (sum_arr == NULL || diff_arr == NULL || mult_arr == NULL || div_arr == NULL) {
        fprintf(stderr, "Memory allocation error for result arrays.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double start, end;

    start = MPI_Wtime();  // Замер времени с помощью MPI
    sum(sum_arr, array1, array2, N);
    diff(diff_arr, array1, array2, N);
    mult(mult_arr, array1, array2, N);
    divv(div_arr, array1, array2, N);
    end = MPI_Wtime();

    printf("Sequential time: %.5f seconds\n", end - start);

    free(sum_arr);
    free(diff_arr);
    free(mult_arr);
    free(div_arr);
    free(array1);
    free(array2);
    
    MPI_Finalize();  // Финализация MPI
    return 0;
}

// Реализации функций операций остаются без изменений
void sum(int *sum_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        sum_arr[i] = arr1[i] + arr2[i];
    }
}

void diff(int *diff_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        diff_arr[i] = arr1[i] - arr2[i];
    }
}

void mult(int *mult_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        mult_arr[i] = arr1[i] * arr2[i];
    }
}

void divv(int *div_arr, int *arr1, int *arr2, int N) {
    for (int i = 0; i < N; i++) {
        div_arr[i] = arr1[i] / arr2[i];
    }
}