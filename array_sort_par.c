#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MIN_PARALLEL_SIZE 10000

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[low];
    int i = low + 1;
    int j = high;

    while(1) {
        while(i <= high && arr[i] <= pivot) i++;
        while(arr[j] > pivot) j--;
        if(i < j) {
            swap(&arr[i], &arr[j]);
        } else {
            swap(&arr[low], &arr[j]);
            return j;
        }
    }
}

void quicksort(int arr[], int low, int high) {
    if(low < high) {
        int j = partition(arr, low, high);

        if(high - low > MIN_PARALLEL_SIZE) {
            #pragma omp task shared(arr)
            quicksort(arr, low, j-1);
            
            #pragma omp task shared(arr)
            quicksort(arr, j+1, high);
            
            #pragma omp taskwait
        } else {
            quicksort(arr, low, j-1);
            quicksort(arr, j+1, high);
        }
    }
}

int main() {
    const int N = 1000000;
    int* arr = malloc(N * sizeof(int));
    srand(time(NULL));

    #pragma omp parallel for
    for(int i=0; i<N; i++)
        arr[i] = rand() % 1000000;

    double start = omp_get_wtime();
    
    #pragma omp parallel
    {
        #pragma omp single
        quicksort(arr, 0, N-1);
    }
    
    double end = omp_get_wtime();

    for(int i=0; i<N-1; i++) {
        if(arr[i] > arr[i+1]) {
            printf("Sorting error!\n");
            break;
        }
    }

    printf("Parallel time: %.5f seconds\n", end - start);
    free(arr);
    return 0;
}