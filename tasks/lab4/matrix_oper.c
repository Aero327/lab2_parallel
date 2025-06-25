#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <time.h>

#define CHECK(call) { \
    const cudaError_t err = call; \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA Error: %s:%d: %s\n", __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
}

__global__ void addKernel(float *a, float *b, float *c, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) c[idx] = a[idx] + b[idx];
}

__global__ void subKernel(float *a, float *b, float *c, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) c[idx] = a[idx] - b[idx];
}

__global__ void mulKernel(float *a, float *b, float *c, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) c[idx] = a[idx] * b[idx];
}

__global__ void divKernel(float *a, float *b, float *c, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) c[idx] = a[idx] / b[idx];
}

void sequential(float *a, float *b, float *c, int size, char op) {
    for (int i = 0; i < size; ++i) {
        switch (op) {
            case '+': c[i] = a[i] + b[i]; break;
            case '-': c[i] = a[i] - b[i]; break;
            case '*': c[i] = a[i] * b[i]; break;
            case '/': c[i] = a[i] / b[i]; break;
        }
    }
}

int main(int argc, char **argv) {
    int n = 100000;

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            n = atoi(argv[i + 1]);
            break;
        }
    }

    if (n <= 0) {
        fprintf(stderr, "Размер массива должен быть больше 0\n");
        return 1;
    }

    size_t bytes = n * sizeof(float);
    float *a = (float *)malloc(bytes);
    float *b = (float *)malloc(bytes);
    float *res_seq = (float *)malloc(bytes);
    float *res_par = (float *)malloc(bytes);

    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++) {
        a[i] = (float)(rand() % 100 + 1);
        b[i] = (float)(rand() % 100 + 1);
    }

    float *d_a, *d_b, *d_c;
    CHECK(cudaMalloc(&d_a, bytes));
    CHECK(cudaMalloc(&d_b, bytes));
    CHECK(cudaMalloc(&d_c, bytes));
    CHECK(cudaMemcpy(d_a, a, bytes, cudaMemcpyHostToDevice));
    CHECK(cudaMemcpy(d_b, b, bytes, cudaMemcpyHostToDevice));

    int blockSize = 256;
    int gridSize = (n + blockSize - 1) / blockSize;

    cudaEvent_t start, stop;
    CHECK(cudaEventCreate(&start));
    CHECK(cudaEventCreate(&stop));
    CHECK(cudaEventRecord(start));

    addKernel<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    subKernel<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    mulKernel<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    divKernel<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);

    CHECK(cudaEventRecord(stop));
    CHECK(cudaEventSynchronize(stop));
    float time_ms = 0;
    CHECK(cudaEventElapsedTime(&time_ms, start, stop));
    CHECK(cudaMemcpy(res_par, d_c, bytes, cudaMemcpyDeviceToHost));

    clock_t start_seq = clock();
    sequential(a, b, res_seq, n, '+');
    sequential(a, b, res_seq, n, '-');
    sequential(a, b, res_seq, n, '*');
    sequential(a, b, res_seq, n, '/');
    clock_t end_seq = clock();
    float time_seq = (float)(end_seq - start_seq) / CLOCKS_PER_SEC;

    printf("Parallel time: %.10f seconds\n", time_ms / 1000.0f);
    printf("Sequential time: %.10f seconds\n", time_seq);

    free(a); free(b); free(res_seq); free(res_par);
    CHECK(cudaFree(d_a)); CHECK(cudaFree(d_b)); CHECK(cudaFree(d_c));
    CHECK(cudaEventDestroy(start)); CHECK(cudaEventDestroy(stop));
    return 0;
}