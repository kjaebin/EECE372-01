#include <arm_neon.h>
#include <asm/unistd.h>        // needed for perf_event syscall
#include <linux/perf_event.h>  // needed for perf_event
#include <math.h>              // needed for floating point routines
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // needed for memset()
#include <sys/ioctl.h>  // needed for ioctl()
#include <sys/time.h>   // needed for gettimeofday()
#include <sys/time.h>
#include <time.h>
#include <unistd.h>  // needed for pid_t type

#include "arm_perf.h"

void func() {
    clock_t np0, np1, p0, p1;

    int16_t* arr1 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t* arr2 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t* ans_neon = malloc(sizeof(int16_t) * 8 * 8);
    int16_t* ans_for = malloc(sizeof(int16_t) * 8 * 8);

    srand((unsigned)time(NULL));
    for (int i = 0; i < 8 * 8; i++) {
        arr1[i] = rand() % 15;
        arr2[i] = rand() % 15;
    }

    ///////////////////////  Matrix multiplication with for loop start /////////////////
    np0 = clock();

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                ans_for[8 * i + j] += arr1[8 * i + k] * arr2[8 * k + j];
            }
        }
    }

    np1 = clock();
    ///////////////////////  Matrix multiplication with for loop end  /////////////////

    ///////// Matrix multiplication with NEON start/////////
    p0 = clock();

    for (int i = 0; i < 8; i++) {
        int16x8_t row1 = vld1q_s16(&arr1[i * 8]);
        for (int j = 0; j < 8; j++) {
            int16x8_t col1 = vdupq_n_s16(arr2[j]);
            int16x8_t col2 = vdupq_n_s16(arr2[8 + j]);
            int16x8_t col3 = vdupq_n_s16(arr2[16 + j]);
            int16x8_t col4 = vdupq_n_s16(arr2[24 + j]);
            int16x8_t col5 = vdupq_n_s16(arr2[32 + j]);
            int16x8_t col6 = vdupq_n_s16(arr2[40 + j]);
            int16x8_t col7 = vdupq_n_s16(arr2[48 + j]);
            int16x8_t col8 = vdupq_n_s16(arr2[56 + j]);

            int16x8_t res = vmulq_s16(row1, col1);
            res = vmlaq_s16(res, row1, col2);
            res = vmlaq_s16(res, row1, col3);
            res = vmlaq_s16(res, row1, col4);
            res = vmlaq_s16(res, row1, col5);
            res = vmlaq_s16(res, row1, col6);
            res = vmlaq_s16(res, row1, col7);
            res = vmlaq_s16(res, row1, col8);

            int16_t sum = 0;
            for (int k = 0; k < 8; k++) {
                sum += vgetq_lane_s16(res, k);
            }

            ans_neon[i * 8 + j] = sum;
        }
    }

    p1 = clock();
    ///////// Matrix multiplication with NEON end///////////

    int check = 0;
    for (int i = 0; i < 8 * 8; i++) {
        if (ans_neon[i] != ans_for[i]) {
            check += 1;
        }
    }
    if (check == 0) {
        printf("PASS\n");
    }
    else {
        printf("FAIL\n");
    }

    printf("Execution time (for) : %7.3lf[us]\n", ((double)np1 - np0) / ((double)CLOCKS_PER_SEC / 1000000));
    printf("Execution time (NEON): %7.3lf[us]\n", ((double)p1 - p0) / ((double)CLOCKS_PER_SEC / 1000000));

    free(arr1);
    free(arr2);
    free(ans_for);
    free(ans_neon);
    return;
}
int main(int argc, char* argv[]) {
    func();

    return 0;
}
