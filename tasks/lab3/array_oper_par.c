#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define MAX_VAL 100

void sum(int *sum_arr, int *arr1, int *arr2, int N);
void diff(int *diff_arr, int *arr1, int *arr2, int N);
void mult(int *mult_arr, int *arr1, int *arr2, int N);
void divv(int *div_arr, int *arr1, int *arr2, int N);

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 100000;
    int opt;

    if (rank == 0) {
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
        srand(time(NULL));
    }

    // Рассылаем размер массива всем процессам
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Вычисление размера локального блока
    int remainder = N % size;
    int chunk_size = N / size;
    int local_count = chunk_size + (rank < remainder ? 1 : 0);

    // Выделение памяти под локальные блоки
    int *local_array1 = (int*)malloc(local_count * sizeof(int));
    int *local_array2 = (int*)malloc(local_count * sizeof(int));
    int *local_sum = (int*)malloc(local_count * sizeof(int));
    int *local_diff = (int*)malloc(local_count * sizeof(int));
    int *local_mult = (int*)malloc(local_count * sizeof(int));
    int *local_div = (int*)malloc(local_count * sizeof(int));
    
    if (!local_array1 || !local_array2 || !local_sum || 
        !local_diff || !local_mult || !local_div) {
        fprintf(stderr, "Process %d: Memory allocation error\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Подготовка данных на корневом процессе
    int *full_array1 = NULL;
    int *full_array2 = NULL;
    int *sendcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        full_array1 = (int*)malloc(N * sizeof(int));
        full_array2 = (int*)malloc(N * sizeof(int));
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        if (!full_array1 || !full_array2 || !sendcounts || !displs) {
            fprintf(stderr, "Root process: Memory allocation error\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Генерация случайных массивов
        for (int i = 0; i < N; i++) {
            full_array1[i] = rand() % MAX_VAL + 1;
            full_array2[i] = rand() % MAX_VAL + 1;
        }

        // Вычисление параметров рассылки
        displs[0] = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = chunk_size + (i < remainder ? 1 : 0);
            if (i > 0) {
                displs[i] = displs[i-1] + sendcounts[i-1];
            }
        }
    }

    // Синхронизация и замер времени
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // Распределение данных
    MPI_Scatterv(full_array1, sendcounts, displs, MPI_INT,
                local_array1, local_count, MPI_INT,
                0, MPI_COMM_WORLD);
                
    MPI_Scatterv(full_array2, sendcounts, displs, MPI_INT,
                local_array2, local_count, MPI_INT,
                0, MPI_COMM_WORLD);

    // Параллельные вычисления над локальными блоками
    sum(local_sum, local_array1, local_array2, local_count);
    diff(local_diff, local_array1, local_array2, local_count);
    mult(local_mult, local_array1, local_array2, local_count);
    divv(local_div, local_array1, local_array2, local_count);

    // Подготовка результирующих массивов (только в root)
    int *global_sum = NULL;
    int *global_diff = NULL;
    int *global_mult = NULL;
    int *global_div = NULL;
    
    if (rank == 0) {
        global_sum = (int*)malloc(N * sizeof(int));
        global_diff = (int*)malloc(N * sizeof(int));
        global_mult = (int*)malloc(N * sizeof(int));
        global_div = (int*)malloc(N * sizeof(int));
    }

    // Сбор результатов
    MPI_Gatherv(local_sum, local_count, MPI_INT,
               global_sum, sendcounts, displs, MPI_INT,
               0, MPI_COMM_WORLD);
               
    MPI_Gatherv(local_diff, local_count, MPI_INT,
               global_diff, sendcounts, displs, MPI_INT,
               0, MPI_COMM_WORLD);
               
    MPI_Gatherv(local_mult, local_count, MPI_INT,
               global_mult, sendcounts, displs, MPI_INT,
               0, MPI_COMM_WORLD);
               
    MPI_Gatherv(local_div, local_count, MPI_INT,
               global_div, sendcounts, displs, MPI_INT,
               0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    double elapsed = end_time - start_time;

    // Освобождение памяти
    free(local_array1);
    free(local_array2);
    free(local_sum);
    free(local_diff);
    free(local_mult);
    free(local_div);
    
    if (rank == 0) {
        free(full_array1);
        free(full_array2);
        free(sendcounts);
        free(displs);
        free(global_sum);
        free(global_diff);
        free(global_mult);
        free(global_div);
        
        printf("Parallel time with %d processes: %.5f seconds\n", size, elapsed);
    }

    MPI_Finalize();
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