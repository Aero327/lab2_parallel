#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <math.h>

#define MAX_VAL 100

void sum(int **sum_arr, int **arr1, int **arr2, int rows, int cols);
void diff(int **diff_arr, int **arr1, int **arr2, int rows, int cols);
void mult(int **mult_arr, int **arr1, int **arr2, int rows, int cols);
void divv(int **div_arr, int **arr1, int **arr2, int rows, int cols);

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    srand(time(NULL) + rank);
    
    int N = 100000;
    int n = 0;
    
    if (rank == 0) {
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
        n = (int)sqrt(N);
    }
    
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (n <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: n must be positive.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }
    
    int *sendbuf1 = NULL;
    int *sendbuf2 = NULL;
    int *sendcounts = NULL;
    int *displs = NULL;
    
    if (rank == 0) {
        sendcounts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        
        int remainder = n % size;
        int offset = 0;
        for (int i = 0; i < size; i++) {
            sendcounts[i] = (n / size) * n;
            if (i < remainder) {
                sendcounts[i] += n;
            }
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }
    
    int local_count;
    MPI_Scatter(sendcounts, 1, MPI_INT, &local_count, 1, MPI_INT, 0, MPI_COMM_WORLD);
    int local_nrows = local_count / n;
    
    if (rank == 0) {
        int **array1 = malloc(n * sizeof(int *));
        int **array2 = malloc(n * sizeof(int *));
        for (int i = 0; i < n; i++) {
            array1[i] = malloc(n * sizeof(int));
            array2[i] = malloc(n * sizeof(int));
            for (int j = 0; j < n; j++) {
                array1[i][j] = rand() % MAX_VAL + 1;
                array2[i][j] = rand() % MAX_VAL + 1;
            }
        }
        
        sendbuf1 = malloc(n * n * sizeof(int));
        sendbuf2 = malloc(n * n * sizeof(int));
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                sendbuf1[i * n + j] = array1[i][j];
                sendbuf2[i * n + j] = array2[i][j];
            }
        }
        
        for (int i = 0; i < n; i++) {
            free(array1[i]);
            free(array2[i]);
        }
        free(array1);
        free(array2);
    }
    
    int *local_array1_data = malloc(local_count * sizeof(int));
    int *local_array2_data = malloc(local_count * sizeof(int));
    
    MPI_Scatterv(sendbuf1, sendcounts, displs, MPI_INT, local_array1_data, local_count, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(sendbuf2, sendcounts, displs, MPI_INT, local_array2_data, local_count, MPI_INT, 0, MPI_COMM_WORLD);
    
    int **local_array1 = malloc(local_nrows * sizeof(int *));
    int **local_array2 = malloc(local_nrows * sizeof(int *));
    for (int i = 0; i < local_nrows; i++) {
        local_array1[i] = local_array1_data + i * n;
        local_array2[i] = local_array2_data + i * n;
    }
    
    int *local_sum_data = malloc(local_count * sizeof(int));
    int *local_diff_data = malloc(local_count * sizeof(int));
    int *local_mult_data = malloc(local_count * sizeof(int));
    int *local_div_data = malloc(local_count * sizeof(int));
    
    int **local_sum = malloc(local_nrows * sizeof(int *));
    int **local_diff = malloc(local_nrows * sizeof(int *));
    int **local_mult = malloc(local_nrows * sizeof(int *));
    int **local_div = malloc(local_nrows * sizeof(int *));
    
    for (int i = 0; i < local_nrows; i++) {
        local_sum[i] = local_sum_data + i * n;
        local_diff[i] = local_diff_data + i * n;
        local_mult[i] = local_mult_data + i * n;
        local_div[i] = local_div_data + i * n;
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    
    sum(local_sum, local_array1, local_array2, local_nrows, n);
    diff(local_diff, local_array1, local_array2, local_nrows, n);
    mult(local_mult, local_array1, local_array2, local_nrows, n);
    divv(local_div, local_array1, local_array2, local_nrows, n);
    
    double end = MPI_Wtime();
    double local_time = end - start;
    double global_time;
    MPI_Reduce(&local_time, &global_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        printf("Parallel time: %.5f seconds\n", global_time);
    }
    
    free(local_array1_data);
    free(local_array2_data);
    free(local_array1);
    free(local_array2);
    free(local_sum_data);
    free(local_diff_data);
    free(local_mult_data);
    free(local_div_data);
    free(local_sum);
    free(local_diff);
    free(local_mult);
    free(local_div);
    
    if (rank == 0) {
        free(sendbuf1);
        free(sendbuf2);
        free(sendcounts);
        free(displs);
    }
    
    MPI_Finalize();
    return 0;
}

void sum(int **sum_arr, int **arr1, int **arr2, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum_arr[i][j] = arr1[i][j] + arr2[i][j];
        }
    }
}

void diff(int **diff_arr, int **arr1, int **arr2, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            diff_arr[i][j] = arr1[i][j] - arr2[i][j];
        }
    }
}

void mult(int **mult_arr, int **arr1, int **arr2, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mult_arr[i][j] = arr1[i][j] * arr2[i][j];
        }
    }
}

void divv(int **div_arr, int **arr1, int **arr2, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            div_arr[i][j] = arr1[i][j] / arr2[i][j];
        }
    }
}