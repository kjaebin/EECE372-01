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
    clock_t np0, np1, p0, p1, p2, p3;

    int16_t *arr1 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *arr2 = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *ans_neon = malloc(sizeof(int16_t) * 8 * 8);
    int16_t *ans_for = malloc(sizeof(int16_t) * 8 * 8);

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
	//make variable
	int16x8_t vec_row[8];
	int16x8_t vec_col[8];
	int16x8_t vec_ans[8];

	for(int i = 0; i < 8; i++){
	    vec_row[i] = vld1q_s16(arr1 + (i * 8));
	    vec_col[i] = vld1q_s16(arr2 + (i * 8));
	    vec_ans[i] = vdupq_n_s16(0);
	}

	int16x8_t temp;

	p0 = clock(); // Operation starts after this line

	//not using for loop
	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 0)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 1)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 2)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 3)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 4)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 5)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 6)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[0], 7)));
	vec_ans[0] = vaddq_s16(vec_ans[0], temp);

	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 0)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 1)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 2)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 3)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 4)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 5)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 6)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[1], 7)));
	vec_ans[1] = vaddq_s16(vec_ans[1], temp);

	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 0)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 1)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 2)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 3)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 4)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 5)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 6)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[2], 7)));
	vec_ans[2] = vaddq_s16(vec_ans[2], temp);

	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 0)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 1)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 2)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 3)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 4)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 5)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 6)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[3], 7)));
	vec_ans[3] = vaddq_s16(vec_ans[3], temp);

	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 0)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 1)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 2)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 3)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 4)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 5)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 6)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[4], 7)));
	vec_ans[4] = vaddq_s16(vec_ans[4], temp);

	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 0)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 1)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 2)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 3)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 4)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 5)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 6)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[5], 7)));
	vec_ans[5] = vaddq_s16(vec_ans[5], temp);

	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 0)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 1)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 2)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 3)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 4)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 5)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 6)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[6], 7)));
	vec_ans[6] = vaddq_s16(vec_ans[6], temp);

	temp = vmulq_s16(vec_col[0], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 0)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
	temp = vmulq_s16(vec_col[1], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 1)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
	temp = vmulq_s16(vec_col[2], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 2)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
	temp = vmulq_s16(vec_col[3], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 3)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
	temp = vmulq_s16(vec_col[4], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 4)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
	temp = vmulq_s16(vec_col[5], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 5)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
	temp = vmulq_s16(vec_col[6], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 6)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
	temp = vmulq_s16(vec_col[7], vdupq_n_s16(vgetq_lane_s16(vec_row[7], 7)));
	vec_ans[7] = vaddq_s16(vec_ans[7], temp);
   

	vst1q_s16(ans_neon, vec_ans[0]);
	vst1q_s16(ans_neon + 8, vec_ans[1]);
	vst1q_s16(ans_neon + 16, vec_ans[2]);
	vst1q_s16(ans_neon + 24, vec_ans[3]);
	vst1q_s16(ans_neon + 32, vec_ans[4]);
	vst1q_s16(ans_neon + 40, vec_ans[5]);
	vst1q_s16(ans_neon + 48, vec_ans[6]);
	vst1q_s16(ans_neon + 56, vec_ans[7]);
	
	p1 = clock();	

    ///////// Matrix multiplication with NEON end///////////
 
	///////// Matrix multiplication with NEON start/////////
	p2 = clock(); // Operation starts after this line

	for (int i = 0; i < 8; i++) {
		int16x8_t row = vld1q_s16(&arr1[i * 8]);
		for (int j = 0; j < 8; j++) {
			int16x8_t col = { arr2[j], arr2[8 + j], arr2[16 + j], arr2[24 + j], arr2[32 + j], arr2[40 + j], arr2[48 + j], arr2[56 + j] };
			int16x8_t prod = vmulq_s16(row, col);

			// Sum the elements of prod vector manually
			int16_t sum = vgetq_lane_s16(prod, 0) + vgetq_lane_s16(prod, 1) + vgetq_lane_s16(prod, 2) +
				vgetq_lane_s16(prod, 3) + vgetq_lane_s16(prod, 4) + vgetq_lane_s16(prod, 5) +
				vgetq_lane_s16(prod, 6) + vgetq_lane_s16(prod, 7);

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
    printf("1Execution time (NEON): %7.3lf[us]\n", ((double)p1 - p0) / ((double)CLOCKS_PER_SEC / 1000000));
	printf("2Execution time (NEON): %7.3lf[us]\n", ((double)p3 - p2) / ((double)CLOCKS_PER_SEC / 1000000));

    free(arr1);
    free(arr2);
    free(ans_for);
    free(ans_neon);
    return;
}
int main(int argc, char *argv[]) {
    func();

    return 0;
}
