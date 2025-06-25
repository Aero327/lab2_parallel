#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>  // Добавляем заголовок MPI

#define MAX_VAL 100

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

    int* array = (int*)malloc(N * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (int i = 0; i < N; i++) {
        array[i] = rand() % MAX_VAL;
    }

    long long sum = 0;
    double start, end;

    start = MPI_Wtime();  // Используем MPI для замера времени
    for (int i = 0; i < N; i++) {
        sum += array[i];
    }
    end = MPI_Wtime();

    printf("Sequential time: %.5f seconds\n", end - start);

    free(array);
    MPI_Finalize();  // Финализация MPI
    return 0;
}