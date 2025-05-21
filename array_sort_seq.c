#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define N 10
#define MAX_VAL 100000

void quick_sort(int *arr, int low, int high);
int partition(int *arr, int low, int high);
void swap(int *a, int *b);
int is_sorted(int *arr, int size);

int main() {
    srand(time(NULL));
    int *array = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        array[i] = rand() % MAX_VAL;
    }

    double start, end;

    start = omp_get_wtime();
    quick_sort(array, 0, N-1);
    end = omp_get_wtime();

    if (!is_sorted(array, N)) {
        printf("Sort error.\n");
    }

    printf("Sequential time: %.5f seconds\n", end - start);

    free(array);
    return 0;
}

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int *arr, int low, int high) {

    // Initialize pivot to be the first element
    int p = arr[low];
    int i = low;
    int j = high;

    while (i < j) {

        // Find the first element greater than
        // the pivot (from starting)
        while (arr[i] <= p && i <= high - 1) {
            i++;
        }

        // Find the first element smaller than
        // the pivot (from last)
        while (arr[j] > p && j >= low + 1) {
            j--;
        }
        if (i < j) {
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[low], &arr[j]);
    return j;
}

void quick_sort(int *arr, int low, int high) {
    if (low < high) {

        // call partition function to find Partition Index
        int pi = partition(arr, low, high);

        // Recursively call quickSort() for left and right
        // half based on Partition Index
        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
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