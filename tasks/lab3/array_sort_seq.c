#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>  // Добавляем заголовок MPI

#define MAX_VAL 1000000  // Добавляем определение MAX_VAL

void swap(int* arr, int i, int j);
void bubbleSort(int arr[], int n);
int is_sorted(int *arr, int size);

int main(int argc, char* argv[]) {
    // Инициализация MPI для последовательного выполнения
    MPI_Init(&argc, &argv);
    
    srand(time(NULL));

    int N = 10000;  // Значение по умолчанию
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

    int *array = (int*)malloc(N * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    for (int i = 0; i < N; i++) {
        array[i] = rand() % MAX_VAL;
    }

    double start, end;

    start = MPI_Wtime();  // Замер времени с помощью MPI
    bubbleSort(array, N);
    end = MPI_Wtime();

    if (!is_sorted(array, N)) {
        printf("Sort error.\n");
    }

    printf("Sequential time: %.5f seconds\n", end - start);

    free(array);
    MPI_Finalize();  // Финализация MPI
    return 0;
}

void swap(int* arr, int i, int j) {
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1])
                swap(arr, j, j + 1);
        }
    }
}

int is_sorted(int *arr, int size) {
    for (int i = 0; i < size-1; i++) {
        if (arr[i] > arr[i+1]) {
            return 0;
        }
    }
    return 1;
}