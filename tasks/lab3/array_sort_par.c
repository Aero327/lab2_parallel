#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#define MAX_VAL 1000000

void bubbleSort(int arr[], int n);
int is_sorted(int *arr, int size);
int compare_and_merge(int **local_arr, int *local_count, int partner_rank, int direction);

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int N = 10000;
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

    // Выделение памяти под локальный массив
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
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        if (!full_array || !sendcounts || !displs) {
            fprintf(stderr, "Root process: Memory allocation error\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        // Генерация случайного массива
        for (int i = 0; i < N; i++) {
            full_array[i] = rand() % MAX_VAL;
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

    // Распределение данных
    MPI_Scatterv(full_array, sendcounts, displs, MPI_INT,
                local_array, local_count, MPI_INT,
                0, MPI_COMM_WORLD);

    // Синхронизация и замер времени
    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    // Локальная сортировка (можно использовать qsort, но для соответствия оставим bubbleSort)
    bubbleSort(local_array, local_count);

    // Параллельная сортировка с использованием чет-нечетной перестановки
    int sorted = 0;
    for (int phase = 0; phase < size; phase++) {
        // Определение партнера для текущей фазы
        int partner;
        int direction; // 0: сохранить меньшие значения, 1: сохранить большие значения
        
        if (phase % 2 == 0) { // Четная фаза
            if (rank % 2 == 0) {
                partner = rank + 1;
                direction = 0; // Сохраняем меньшие значения
            } else {
                partner = rank - 1;
                direction = 1; // Сохраняем большие значения
            }
        } else { // Нечетная фаза
            if (rank % 2 == 1) {
                partner = rank + 1;
                direction = 0; // Сохраняем меньшие значения
            } else {
                partner = rank - 1;
                direction = 1; // Сохраняем большие значения
            }
        }

        // Если партнер существует, выполняем обмен и слияние
        if (partner >= 0 && partner < size) {
            sorted = compare_and_merge(&local_array, &local_count, partner, direction);
        }
        
        // Синхронизация между фазами
        MPI_Barrier(MPI_COMM_WORLD);
    }

    double end_time = MPI_Wtime();
    double elapsed = end_time - start_time;

    // Сбор результатов на корневом процессе для проверки
    int *global_array = NULL;
    if (rank == 0) {
        global_array = (int*)malloc(N * sizeof(int));
    }
    
    MPI_Gatherv(local_array, local_count, MPI_INT,
               global_array, sendcounts, displs, MPI_INT,
               0, MPI_COMM_WORLD);

    // Проверка сортировки и вывод результатов
    if (rank == 0) {
        if (!is_sorted(global_array, N)) {
            printf("Sort error.\n");
        } else {
            printf("Parallel time with %d processes: %.5f seconds\n", size, elapsed);
        }
    }

    // Освобождение памяти
    free(local_array);
    if (rank == 0) {
        free(full_array);
        free(sendcounts);
        free(displs);
        free(global_array);
    }

    MPI_Finalize();
    return 0;
}

// Функция для сравнения и слияния с партнером
int compare_and_merge(int **local_arr, int *local_count, int partner_rank, int direction) {
    // Обмен размерами массивов
    int partner_count;
    MPI_Sendrecv(local_count, 1, MPI_INT, partner_rank, 0,
                 &partner_count, 1, MPI_INT, partner_rank, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Выделение памяти для массива партнера
    int *partner_arr = (int*)malloc(partner_count * sizeof(int));
    if (partner_arr == NULL) {
        fprintf(stderr, "Memory allocation error for partner array\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Обмен массивами
    MPI_Sendrecv(*local_arr, *local_count, MPI_INT, partner_rank, 0,
                 partner_arr, partner_count, MPI_INT, partner_rank, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Создание временного массива для слияния
    int total_count = *local_count + partner_count;
    int *merged = (int*)malloc(total_count * sizeof(int));
    if (merged == NULL) {
        fprintf(stderr, "Memory allocation error for merged array\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Слияние двух отсортированных массивов
    int i = 0, j = 0, k = 0;
    while (i < *local_count && j < partner_count) {
        if ((*local_arr)[i] < partner_arr[j]) {
            merged[k++] = (*local_arr)[i++];
        } else {
            merged[k++] = partner_arr[j++];
        }
    }
    while (i < *local_count) merged[k++] = (*local_arr)[i++];
    while (j < partner_count) merged[k++] = partner_arr[j++];

    // Определение части, которую нужно сохранить
    int new_count = (direction == 0) ? *local_count : partner_count;
    int start_index = (direction == 0) ? 0 : total_count - new_count;

    // Освобождаем старый массив и выделяем память для нового
    free(*local_arr);
    *local_arr = (int*)malloc(new_count * sizeof(int));
    if (*local_arr == NULL) {
        fprintf(stderr, "Memory allocation error for new local array\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Копируем нужную часть
    for (int idx = 0; idx < new_count; idx++) {
        (*local_arr)[idx] = merged[start_index + idx];
    }
    
    // Обновляем счетчик
    *local_count = new_count;

    // Освобождение временной памяти
    free(partner_arr);
    free(merged);

    return 1;
}

// Пузырьковая сортировка
void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// Проверка отсортированности массива
int is_sorted(int *arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0;
        }
    }
    return 1;
}