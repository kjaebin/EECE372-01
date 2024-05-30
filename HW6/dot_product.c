#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "omp.h"

#define ARRAY_SIZE 100000

double dotp(double *x, double *y);
double dotp_omp(double *x, double *y);

int main() {
    double error = 0;
    double global_sum = 0;
    double global_sum_ref = 0;
    double *x = malloc(sizeof(double) * ARRAY_SIZE);
    double *y = malloc(sizeof(double) * ARRAY_SIZE);
    double *z = malloc(sizeof(double) * ARRAY_SIZE);

    clock_t start, end;

    srand((unsigned)time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        z[i] = 0;
        x[i] = rand() % 10 + 1;
        y[i] = rand() % 10 + 1;
    }

    start = clock();
    global_sum = dotp(x, y);
    printf("Execution time (dot product)    : %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    start = clock();
    global_sum = dotp_omp(x, y);
    printf("Execution time (dot product omp): %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_sum_ref += x[i] * y[i];
    }

    error = global_sum - global_sum_ref;
    if (error == 0) {
        printf("PASS\n");
    }
    else {
        printf("FAIL\n");
    }

    free(x);
    free(y);
    free(z);
    return 0;
}

double dotp(double *x, double *y) {
    double global_sum = 0.0;

    for (int i = 0; i < ARRAY_SIZE; i++) {
        global_sum += x[i] * y[i];
    }

    return global_sum;
}

double dotp_omp(double *x, double *y) {
    omp_set_num_threads(6);

    double global_sum = 0.0;

#pragma omp parallel
    {
        double local_sum = 0.0;
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        for (int i = thread_id; i < ARRAY_SIZE; i += num_threads) {
            local_sum += x[i] * y[i];
        }

#pragma omp critical
        {
            global_sum += local_sum;
        }
    }

    return global_sum;
}
