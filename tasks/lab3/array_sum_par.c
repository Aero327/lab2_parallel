#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define MAX_VAL 100

int main(int argc, char* argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int N = 100000;
    int opt;

    // Обработка аргументов командной строки только на корневом процессе
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
        srand(time(NULL)); // Инициализация генератора случайных чисел
    }

    // Рассылаем размер массива всем процессам
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Вычисление размера локального блока
    int remainder = N % size;
    int chunk_size = N / size;
    int local_count = chunk_size + (rank < remainder ? 1 : 0);

    // Выделение памяти под локальный блок
    int *local_array = (int*)malloc(local_count * sizeof(int));
    if (local_array == NULL) {
        fprintf(stderr, "Process %d: Memory allocation error\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Подготовка данных на корневом процессе
    int *full_array = NULL;
    int *sendcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        full_array = (int*)malloc(N * sizeof(int));
        if (full_array == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Генерация случайного массива
        for (int i = 0; i < N; i++) {
            full_array[i] = rand() % MAX_VAL;
        }

        // Вычисление параметров рассылки
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        if (sendcounts == NULL || displs == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

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
    MPI_Scatterv(full_array, sendcounts, displs, MPI_INT,
                local_array, local_count, MPI_INT,
                0, MPI_COMM_WORLD);

    // Локальное суммирование
    long long local_sum = 0;
    for (int i = 0; i < local_count; i++) {
        local_sum += local_array[i];
    }

    // Сбор результатов на корневом процессе
    long long global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    double elapsed = end_time - start_time;

    // Освобождение памяти
    free(local_array);
    if (rank == 0) {
        free(full_array);
        free(sendcounts);
        free(displs);
    }

    // Вывод результатов
    if (rank == 0) {
        printf("Parallel time with %d processes: %.5f seconds\n", size, elapsed);
    }

    MPI_Finalize();
    return 0;
}