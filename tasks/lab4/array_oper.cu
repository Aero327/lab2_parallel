#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <unistd.h>
#include <time.h>

#define CHECK(call) \
{ \
    const cudaError_t error = call; \
    if (error != cudaSuccess) \
    { \
        fprintf(stderr, "CUDA Error: %s:%d, ", __FILE__, __LINE__); \
        fprintf(stderr, "code: %d, reason: %s\n", error, cudaGetErrorString(error)); \
        exit(EXIT_FAILURE); \
    } \
}

__global__ void array_operations_kernel_cuda(double* a, double* b, double* sum,
                                       double* diff, double* prod, double* quot, int N_chunk) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N_chunk) {
        sum[idx] = a[idx] + b[idx];
        diff[idx] = a[idx] - b[idx];
        prod[idx] = a[idx] * b[idx];
        quot[idx] = (b[idx] != 0.0) ? a[idx] / b[idx] : 0.0;
    }
}

void array_operations_sequential(double* a, double* b, double* sum,
                                double* diff, double* prod, double* quot, int N_total) {
    for (int i = 0; i < N_total; ++i) {
        sum[i] = a[i] + b[i];
        diff[i] = a[i] - b[i];
        prod[i] = a[i] * b[i];
        quot[i] = (b[i] != 0.0) ? a[i] / b[i] : 0.0;
    }
}


void process_in_chunks_cuda(double* a, double* b, double* sum, double* diff, double* prod, double* quot, int N_total, int chunk_size, int threadsPerBlock) {
    double *d_a, *d_b, *d_sum, *d_diff, *d_prod, *d_quot;

    CHECK(cudaMalloc((void**)&d_a, chunk_size * sizeof(double)));
    CHECK(cudaMalloc((void**)&d_b, chunk_size * sizeof(double)));
    CHECK(cudaMalloc((void**)&d_sum, chunk_size * sizeof(double)));
    CHECK(cudaMalloc((void**)&d_diff, chunk_size * sizeof(double)));
    CHECK(cudaMalloc((void**)&d_prod, chunk_size * sizeof(double)));
    CHECK(cudaMalloc((void**)&d_quot, chunk_size * sizeof(double)));

    int blocksPerGrid = (chunk_size + threadsPerBlock - 1) / threadsPerBlock;

    for (int i = 0; i < N_total; i += chunk_size) {
        int current_chunk_size = (i + chunk_size > N_total) ? (N_total - i) : chunk_size;

        CHECK(cudaMemcpy(d_a, a + i, current_chunk_size * sizeof(double), cudaMemcpyHostToDevice));
        CHECK(cudaMemcpy(d_b, b + i, current_chunk_size * sizeof(double), cudaMemcpyHostToDevice));

        array_operations_kernel_cuda<<<blocksPerGrid, threadsPerBlock>>>(d_a, d_b, d_sum, d_diff, d_prod, d_quot, current_chunk_size);
        CHECK(cudaGetLastError());

        CHECK(cudaMemcpy(sum + i, d_sum, current_chunk_size * sizeof(double), cudaMemcpyDeviceToHost));
        CHECK(cudaMemcpy(diff + i, d_diff, current_chunk_size * sizeof(double), cudaMemcpyDeviceToHost));
        CHECK(cudaMemcpy(prod + i, d_prod, current_chunk_size * sizeof(double), cudaMemcpyDeviceToHost));
        CHECK(cudaMemcpy(quot + i, d_quot, current_chunk_size * sizeof(double), cudaMemcpyDeviceToHost));
    }

    CHECK(cudaFree(d_a));
    CHECK(cudaFree(d_b));
    CHECK(cudaFree(d_sum));
    CHECK(cudaFree(d_diff));
    CHECK(cudaFree(d_prod));
    CHECK(cudaFree(d_quot));
}

int main(int argc, char *argv[]) {
    int N = 1000000;
    int threadsPerBlock = 256;
    int chunk_size = 1000000;

    int opt;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                N = atoi(optarg);
                if (N <= 0) {
                    fprintf(stderr, "Error: Array size (N) must be a positive integer.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                fprintf(stderr, "Usage: %s [-n array_size]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (N < chunk_size) {
        chunk_size = N;
    }

    printf("Processing array of size N = %d\n", N);
    if (N >= chunk_size) { // Only print chunk_size if it's relevant
        printf("Using chunk_size = %d (for CUDA), threadsPerBlock = %d\n", chunk_size, threadsPerBlock);
    } else {
        printf("CUDA chunking not applicable for N < default chunk_size. N will be used as chunk_size.\n");
    }


    double *a, *b;
    double *sum_seq, *diff_seq, *prod_seq, *quot_seq;
    double *sum_par, *diff_par, *prod_par, *quot_par;

    a = (double*)malloc(N * sizeof(double));
    b = (double*)malloc(N * sizeof(double));

    sum_seq = (double*)malloc(N * sizeof(double));
    diff_seq = (double*)malloc(N * sizeof(double));
    prod_seq = (double*)malloc(N * sizeof(double));
    quot_seq = (double*)malloc(N * sizeof(double));

    sum_par = (double*)malloc(N * sizeof(double));
    diff_par = (double*)malloc(N * sizeof(double));
    prod_par = (double*)malloc(N * sizeof(double));
    quot_par = (double*)malloc(N * sizeof(double));


    if (!a || !b || !sum_seq || !diff_seq || !prod_seq || !quot_seq ||
        !sum_par || !diff_par || !prod_par || !quot_par) {
        perror("Memory allocation failed");
        free(a); free(b);
        free(sum_seq); free(diff_seq); free(prod_seq); free(quot_seq);
        free(sum_par); free(diff_par); free(prod_par); free(quot_par);
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        a[i] = (double)rand() / RAND_MAX * 100.0 + 1.0;
        b[i] = (double)rand() / RAND_MAX * 100.0 + 1.0;
    }

    clock_t start_seq_time, end_seq_time;

    start_seq_time = clock();
    array_operations_sequential(a, b, sum_seq, diff_seq, prod_seq, quot_seq, N);
    end_seq_time = clock();
    double sequential_cpu_time = (double)(end_seq_time - start_seq_time) / CLOCKS_PER_SEC;
    printf("Sequential time: %.10f seconds\n", sequential_cpu_time);


    cudaEvent_t start_event, stop_event;
    CHECK(cudaEventCreate(&start_event));
    CHECK(cudaEventCreate(&stop_event));

    CHECK(cudaEventRecord(start_event, 0));
    process_in_chunks_cuda(a, b, sum_par, diff_par, prod_par, quot_par, N, chunk_size, threadsPerBlock);
    CHECK(cudaEventRecord(stop_event, 0));
    CHECK(cudaEventSynchronize(stop_event));

    float milliseconds_cuda = 0;
    CHECK(cudaEventElapsedTime(&milliseconds_cuda, start_event, stop_event));
    printf("Parallel time (CUDA): %.10f seconds\n", milliseconds_cuda / 1000.0f);

    CHECK(cudaEventDestroy(start_event));
    CHECK(cudaEventDestroy(stop_event));

    free(a); free(b);
    free(sum_seq); free(diff_seq); free(prod_seq); free(quot_seq);
    free(sum_par); free(diff_par); free(prod_par); free(quot_par);

    printf("Processing finished.\n");
    return 0;
}