#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

#define MAX_VAL 100

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

    int* array = (int*)malloc(N * sizeof(int));
    
    if (array == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        return 1;
    }

    for (int i = 0; i < N; i++) {
        array[i] = rand() % MAX_VAL;
    }

    long long sum = 0;
    double start, end;

    start = omp_get_wtime();
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < N; i++) {
        sum += array[i];
    }
    end = omp_get_wtime();

    printf("Parallel time: %.5f seconds\n", end - start);

    free(array);
    return 0;
}