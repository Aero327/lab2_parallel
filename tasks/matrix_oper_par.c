#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#include <unistd.h>

#define MAX_VAL 100

void sum(int **sum_arr, int **arr1, int **arr2, int N);
void diff(int **diff_arr, int **arr1, int **arr2, int N);
void mult(int **mult_arr, int **arr1, int **arr2, int N);
void divv(int **div_arr, int **arr1, int **arr2, int N);

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

    int **array1 = malloc((int)sqrt(N) * sizeof(int *));
	int **array2 = malloc((int)sqrt(N) * sizeof(int *));
    int **sum_arr = malloc((int)sqrt(N) * sizeof(int *));
    int **diff_arr = malloc((int)sqrt(N) * sizeof(int *));
    int **mult_arr = malloc((int)sqrt(N) * sizeof(int *));
    int **div_arr = malloc((int)sqrt(N) * sizeof(int *));
    
    for(int i = 0; i < (int)sqrt(N); i++) {
		array1[i] = malloc((int)sqrt(N) * sizeof(int));
        array2[i] = malloc((int)sqrt(N) * sizeof(int));
        sum_arr[i] = malloc((int)sqrt(N) * sizeof(int));
        diff_arr[i] = malloc((int)sqrt(N) * sizeof(int));
        mult_arr[i] = malloc((int)sqrt(N) * sizeof(int));
        div_arr[i] = malloc((int)sqrt(N) * sizeof(int));
    }
    
    for (int i = 0; i < (int)sqrt(N); i++) {
        for (int j = 0; j < (int)sqrt(N); j++) {
            array1[i][j] = rand() % MAX_VAL + 1;
            array2[i][j] = rand() % MAX_VAL + 1;
        }
    }

    double start, end;

    start = omp_get_wtime();
    #pragma omp parallel sections
    {
        #pragma omp section
        { sum(sum_arr, array1, array2, (int)sqrt(N)); }
        
        #pragma omp section
        { diff(diff_arr, array1, array2, (int)sqrt(N)); }
        
        #pragma omp section
        { mult(mult_arr, array1, array2, (int)sqrt(N)); }
        
        #pragma omp section
        { divv(div_arr, array1, array2, (int)sqrt(N)); }
    }
    end = omp_get_wtime();

    printf("Parallel time: %.5f seconds\n", end - start);

    for(int i = 0; i < (int)sqrt(N); i++) {
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

void sum(int **sum_arr, int **arr1, int **arr2, int N) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum_arr[i][j] = arr1[i][j] + arr2[i][j];
        }
    }
}

void diff(int **diff_arr, int **arr1, int **arr2, int N) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            diff_arr[i][j] = arr1[i][j] - arr2[i][j];
        }
    }
}

void mult(int **mult_arr, int **arr1, int **arr2, int N) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            mult_arr[i][j] = arr1[i][j] * arr2[i][j];
        }
    }
}

void divv(int **div_arr, int **arr1, int **arr2, int N) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            div_arr[i][j] = arr1[i][j] / arr2[i][j];
        }
    }
}