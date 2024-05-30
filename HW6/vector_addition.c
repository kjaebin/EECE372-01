#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "omp.h"

#define ARRAY_SIZE 1000000

void vec_simple(double* x, double* y, double* z);
void vec_slicing(double* x, double* y, double* z);
void vec_chunking(double* x, double* y, double* z);

int main() {
    double error1 = 0;
    double error2 = 0;
    double* x = malloc(sizeof(double) * ARRAY_SIZE);
    double* y = malloc(sizeof(double) * ARRAY_SIZE);
    double* z = malloc(sizeof(double) * ARRAY_SIZE);

    clock_t start, end;

    srand((unsigned)time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        z[i] = 0;
        x[i] = rand() % 10 + 1;
        y[i] = rand() % 10 + 1;
    }

    start = clock();
    vec_simple(x, y, z);
    printf("Execution time (simple)  : %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    start = clock();
    vec_slicing(x, y, z);
    printf("Execution time (slicing) : %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        error1 = error1 + (z[i] - (x[i] + y[i]));
    }

    start = clock();
    vec_chunking(x, y, z);
    printf("Execution time (chunking): %7.3lf[ms]\n", ((double)clock() - start) / ((double)CLOCKS_PER_SEC / 1000));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        error2 = error2 + (z[i] - (x[i] + y[i]));
    }

    if (error1 == 0 && error2 == 0) {
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

void vec_simple(double* x, double* y, double* z) {
    omp_set_num_threads(6);
#pragma omp parallel
    {
        for (int i = 0; i < ARRAY_SIZE; i++) {
            z[i] = x[i] + y[i];
        }
    }
}

void vec_slicing(double* x, double* y, double* z) {
    // 스레드 수를 6으로 설정
    omp_set_num_threads(6);

    // 병렬 영역 시작
    #pragma omp parallel
    {
        // 현재 스레드 ID와 총 스레드 수를 얻음.
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        // 각 스레드는 자신에게 할당된 인덱스에서 시작하여 num_threads만큼 건너뛰며 벡터 덧셈을 수행함.
        for (int i = thread_id; i < ARRAY_SIZE; i += num_threads) {
            z[i] = x[i] + y[i];
        }
    }
}

void vec_chunking(double* x, double* y, double* z) {
    // 스레드 수를 6으로 설정
    omp_set_num_threads(6);

    // 병렬 영역 시작
    #pragma omp parallel
    {
        // 현재 스레드 ID와 총 스레드 수를 얻음
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        // 각 스레드가 처리할 청크의 크기 계산
        int chunk_size = ARRAY_SIZE / num_threads;

        // 각 스레드가 처리할 시작 인덱스와 끝 인덱스를 계산
        int start_index = thread_id * chunk_size;
        int end_index;

        // 마지막 스레드는 나머지 요소들을 처리
        if (thread_id == num_threads - 1) {
            end_index = ARRAY_SIZE;
        }
        else {
            end_index = start_index + chunk_size;
        }

        // 각 스레드는 자신에게 할당된 범위 내에서 벡터 덧셈 수행
        for (int i = start_index; i < end_index; i++) {
            z[i] = x[i] + y[i];
        }
    }
}
