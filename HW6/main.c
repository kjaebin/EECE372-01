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
	p2 = clock();

	for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int16x8_t sum_vec = vdupq_n_s16(0);  // Initialize sum vector to zero
            for (int k = 0; k < 8; k++) {
                int16x8_t a_vec = vdupq_n_s16(arr1[i * 8 + k]);
                int16x8_t b_vec = vld1q_s16(&arr2[k * 8]);
                int16x8_t product = vmulq_s16(a_vec, b_vec);
                sum_vec = vaddq_s16(sum_vec, product);
            }
            int16_t sum = 0;
            for (int l = 0; l < 8; l++) {
                sum += vgetq_lane_s16(sum_vec, l);
            }
            ans_neon[i * 8 + j] = sum;
        }
    }

	p3 = clock();
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
