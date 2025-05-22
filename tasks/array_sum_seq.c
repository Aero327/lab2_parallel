#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define N 1000000
#define MAX_VAL 100

int main() {
    srand(time(NULL));

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
    for (int i = 0; i < N; i++) {
        sum += array[i];
    }
    end = omp_get_wtime();

    printf("Sequential time: %.5f seconds\n", end - start);

    free(array);
    return 0;
}