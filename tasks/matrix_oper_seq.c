#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#define MAX_VAL 100

void sum(int **sum_arr, int **arr1, int **arr2);
void diff(int **diff_arr, int **arr1, int **arr2);
void mult(int **mult_arr, int **arr1, int **arr2);
void divv(int **div_arr, int **arr1, int **arr2);

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

    int **array1 = malloc(N * sizeof(int *));
	int **array2 = malloc(N * sizeof(int *));
    int **sum_arr = malloc(N * sizeof(int *));
    int **diff_arr = malloc(N * sizeof(int *));
    int **mult_arr = malloc(N * sizeof(int *));
    int **div_arr = malloc(N * sizeof(int *));
    
    for(int i = 0; i < N; i++) {
		array1[i] = malloc(N * sizeof(int));
        array2[i] = malloc(N * sizeof(int));
        sum_arr[i] = malloc(N * sizeof(int));
        diff_arr[i] = malloc(N * sizeof(int));
        mult_arr[i] = malloc(N * sizeof(int));
        div_arr[i] = malloc(N * sizeof(int));
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            array1[i][j] = rand() % MAX_VAL + 1;
            array2[i][j] = rand() % MAX_VAL + 1;
        }
    }

    double start, end;

    start = omp_get_wtime();
    sum(sum_arr, array1, array2);
    diff(diff_arr, array1, array2);
    mult(mult_arr, array1, array2);
    divv(div_arr, array1, array2);
    end = omp_get_wtime();

    printf("Sequential time: %.5f seconds\n", end - start);

    for(int i = 0; i < N; i++) {
        free(array1[i]);
        free(array2[i]);
        free(sum_arr[i]);
        free(diff_arr[i]);
        free(mult_arr[i]);
        free(div_arr[i]);
    }
    free(array1);
    free(array2);
    free(sum_arr);
    free(mult_arr);
    free(diff_arr);
    free(div_arr);

    return 0;
}

void sum(int **sum_arr, int **arr1, int **arr2) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum_arr[i][j] = arr1[i][j] + arr2[i][j];
        }
    }
}

void diff(int **diff_arr, int **arr1, int **arr2) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            diff_arr[i][j] = arr1[i][j] - arr2[i][j];
        }
    }
}

void mult(int **mult_arr, int **arr1, int **arr2) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mult_arr[i][j] = arr1[i][j] * arr2[i][j];
        }
    }
}

void divv(int **div_arr, int **arr1, int **arr2) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            div_arr[i][j] = arr1[i][j] / arr2[i][j];
        }
    }
}