#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function prototypes
void mergesort_C(int* a, int low, int high);
void merge_C(int* a, int low, int mid, int high);
void mergesort_ASM(int* a, int low, int high);
void merge_ASM(int* a, int low, int mid, int high);

// Helper function to print arrays
void printArray(int* a, int size);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    // user input
    if (argc != 2) {
        printf("Usage: %s <number of elements>\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);  // Number of elements in array

    if (n <= 0) {
        printf("Number of elements must be positive.\n");
        return 1;
    }

    int* data = (int*)malloc(sizeof(int) * n);
    int* data_asm = (int*)malloc(sizeof(int) * n);

    if (!data || !data_asm) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // variable intialization
    int i = 0;
    for (i = 0; i < n; i++) data[i] = i + 1;

    // Shuffle the elements of array randomly
    for (i = 0; i < n - 1; i++) {
        int r = rand() % (n - i) + i;
        int temp = data[i];
        data[i] = data[r];
        data[r] = temp;
    }
    for (i = 0; i < n; i++) data_asm[i] = data[i];   // Copy to assembly data array

    // print data before sorting
    if (n <= 20) {
        printf("Before sort     : [ ");
        for (int i = 0; i < n; i++) {
            printf("%d ", data[i]);
        }
        printf("]\n");
    }

    // Time measurement setup
    clock_t begin1, end1;
    clock_t begin2, end2;

    // Sorting with C implementation
    begin1 = clock();
    mergesort_C(data, 0, n - 1);
    end1 = clock();
    float elapsed_c = (float)(end1 - begin1) / CLOCKS_PER_SEC;

    // Sorting with Assembly implementation
    begin2 = clock();
    mergesort_ASM(data_asm, 0, n - 1);
    end2 = clock();
    float elapsed_asm = (float)(end2 - begin2) / CLOCKS_PER_SEC;

    // print data after sorting
    if (n <= 20) {
        printf("After sort   (C): [ ");
        for (int i = 0; i < n; i++) {
            printf("%d ", data[i]);
        }
        printf("]\n");

        printf("After sort (ASM): [ ");
        for (int i = 0; i < n; i++) {
            printf("%d ", data_asm[i]);
        }
        printf("]\n");
    }

    // print run time
    printf("Execution Time   (C): %f [sec]\n", elapsed_c);
    printf("Execution Time (ASM): %f [sec]\n", elapsed_asm);

    free(data);
    free(data_asm);

    return 0;
}

void mergesort_C(int* a, int low, int high) {
    if (low < high) { // Only proceed if there are at least two elements to sort
        int mid = low + (high - low) / 2; // Find the midpoint to avoid overflow

        // Recursively sort the left half
        mergesort_C(a, low, mid);

        // Recursively sort the right half
        mergesort_C(a, mid + 1, high);

        // Merge the sorted halves
        merge_C(a, low, mid, high);
    }
}

void merge_C(int* a, int low, int mid, int high) {
    int n = high - low + 1; // Number of elements to merge
    int* temp = (int*)malloc(n * sizeof(int)); // Temporary array for merging
    int leftIndex = low, rightIndex = mid + 1, tempIndex = 0;

    // Merge the two sorted halves into a temporary array
    while (leftIndex <= mid && rightIndex <= high) {
        if (a[leftIndex] <= a[rightIndex]) {
            temp[tempIndex++] = a[leftIndex++];
        }
        else {
            temp[tempIndex++] = a[rightIndex++];
        }
    }

    // Copy any remaining elements from the left half
    while (leftIndex <= mid) {
        temp[tempIndex++] = a[leftIndex++];
    }

    // Copy any remaining elements from the right half
    while (rightIndex <= high) {
        temp[tempIndex++] = a[rightIndex++];
    }

    // Copy the sorted elements back into the original array
    for (int i = 0; i < n; i++) {
        a[low + i] = temp[i];
    }

    free(temp); // Free the temporary array
}

void mergesort_ASM(int* a, int low, int high) {
    asm(
        // Initialize registers r4, r5 only if they are not set (can use a flag or check if r4, r5 are zero)
        "cmp r1, #0\n\t"
        "cmpeq r2, #0\n\t"
        "moveq r1, %[l]\n\t"               // Set r4 to low if not already set
        "moveq r2, %[h]\n\t"               // Set r5 to high if not already set
        
        // 비교 연산을 수행하여 low와 high를 비교
        "mov r0, %[a]\n\t"                 // 배열 포인터 a를 r0에 설정
        "cmp r1, r2\n\t"               // 비교: low >= high
        "bge end_mergesort\n\t"            // 만약 low >= high 이면, 재귀의 베이스 케이스에 도달했으므로 end_mergesort로 분기

        // 중간 지점 계산
        "sub r3, r2, r1\n\t"           // r3 = high - low, 배열 길이 계산
        "lsr r3, r3, #1\n\t"               // r3 = (high - low) / 2, 오른쪽으로 한 비트 시프트하여 2로 나눔
        "add r3, r1, r3\n\t"             // r3 = low + (high - low) / 2, 중간 인덱스 계산

        // 첫 번째 재귀 호출: 왼쪽 부분 배열 정렬
        "push {r0-r3, lr}\n\t"             // r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 스택에 저장
        "mov r2, r3\n\t"                   // 중간 인덱스 mid를 r2에 설정
        "bl mergesort_ASM\n\t"             // mergesort_ASM 함수 호출
        "pop {r0-r3, lr}\n\t"              // 스택에서 r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 복구

        // 두 번째 재귀 호출: 오른쪽 부분 배열 정렬
        "push {r0-r3, lr}\n\t"             // r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 스택에 저장
        "mov r1, r3\n\t"                   // 중간 인덱스 mid를 r1에 다시 설정
        "add r1, r1, #1\n\t"               // r1 = mid + 1, 오른쪽 부분 배열의 시작 인덱스 설정
        "mov r2, %[h]\n\t"                 // 종료 인덱스 high를 r2에 설정
        "bl mergesort_ASM\n\t"             // mergesort_ASM 함수 호출
        "pop {r0-r3, lr}\n\t"              // 스택에서 r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 복구

        // 병합 호출: 정렬된 두 부분 배열 병합
        "push {r0-r3, lr}\n\t"             // r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 스택에 저장
        "mov r3, r2\n\t"                   // 종료 인덱스 high를 r3에 설정
        "mov r2, r3\n\t"                   // 중간 인덱스 mid를 r2에 설정
        "push {r0-r3, lr}\n\t"             // r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 스택에 저장
        "bl merge_ASM\n\t"                 // merge_ASM 함수 호출
        "pop {r0-r3, lr}\n\t"              // 스택에서 r0부터 r3까지의 레지스터와 링크 레지스터(lr)를 복구

        "end_mergesort:\n\t"               // 재귀의 베이스 케이스 및 함수 종료 지점 레이블
        :
        : [a] "r" (a), [l] "r" (low), [h] "r" (high)  // 입력: 배열 포인터, 시작 인덱스, 종료 인덱스
        : "r0", "r1", "r2", "r3", "lr", "memory", "cc"  // clobbered: 사용된 레지스터와 메모리, 조건 코드
        );
}

void merge_ASM(int* a, int low, int mid, int high) {
    int leftIndex = low, rightIndex = mid + 1, tempIndex = 0;
    int* temp = (int*)malloc((high - low + 1) * sizeof(int)); // 임시 배열을 위한 메모리 할당
    int n = high - low + 1; // Number of elements to merge

    asm(
        // 초기 레지스터 설정
        "mov %[li], %[low]\n\t"            // leftIndex를 low 값으로 초기화
        "mov %[ri], %[mid]\n\t"            // rightIndex를 mid 값으로 초기화
        "add %[ri], %[ri], #1\n\t"         // rightIndex를 mid+1로 설정하여 오른쪽 부분 배열의 시작점으로 설정
        "mov %[ti], #0\n\t"                // tempIndex를 0으로 초기화

        // 병합 루프 시작
        "loop_merge:\n\t"
        "cmp %[li], %[mid]\n\t"            // leftIndex와 mid 비교
        "bgt left_done\n\t"                 // leftIndex가 mid보다 크면 left 부분 배열이 끝났음을 의미, left_done으로 점프
        "cmp %[ri], %[high]\n\t"           // rightIndex와 high 비교
        "bgt right_done\n\t"                // rightIndex가 high보다 크면 right 부분 배열이 끝났음을 의미, right_done으로 점프

        // 두 부분 배열의 현재 요소를 로드
        "ldr r5, [%[a], %[li], LSL #2]\n\t"  // a[leftIndex]의 값을 r5에 로드
        "ldr r6, [%[a], %[ri], LSL #2]\n\t"  // a[rightIndex]의 값을 r6에 로드

        // 비교 및 temp에 저장
        "cmp r5, r6\n\t"
        "bgt copy_right\n\t"                // r5 > r6 이면 오른쪽 요소를 temp에 복사

        "copy_left:\n\t"
        "str r5, [%[temp], %[ti], LSL #2]\n\t"  // temp[tempIndex]에 a[leftIndex]값 저장
        "add %[li], %[li], #1\n\t"         // leftIndex 증가
        "b increment_temp\n\t"             // tempIndex 증가로 점프

        "copy_right:\n\t"
        "str r6, [%[temp], %[ti], LSL #2]\n\t"  // temp[tempIndex]에 a[rightIndex]값 저장
        "add %[ri], %[ri], #1\n\t"         // rightIndex 증가

        "increment_temp:\n\t"
        "add %[ti], %[ti], #1\n\t"         // tempIndex 증가
        "b loop_merge\n\t"                 // 병합 루프로 돌아가기

        "left_done:\n\t"
        "right_done:\n\t"
        "check_left:\n\t"
        // 남은 왼쪽 부분 배열 요소를 temp에 복사
        // leftIndex가 mid 보다 크지 않은 경우 (즉, 아직 왼쪽 부분 배열에 요소가 남아있는 경우) 계속 진행
        "cmp %[li], %[mid]\n\t"
        "bgt end_left\n\t"           // rightIndex가 high보다 크면 루프를 종료하고 merge 작업을 마무리
        "ldr r5, [%[a], %[li], LSL #2]\n\t" // 업데이트된 a[leftIndex]의 값을 r5에 로드
        "str r5, [%[temp], %[ti], LSL #2]\n\t" // r5 레지스터의 값을 temp[tempIndex]에 저장
        "add %[li], %[li], #1\n\t"       // leftIndex 증가
        "add %[ti], %[ti], #1\n\t"       // tempIndex 증가
        "b check_left\n\t"                 // 다시 check_left 레이블로 점프하여 남은 오른쪽 요소를 계속 복사

        "end_left:\n\t"
        "check_right:\n\t"
        // 남은 오른쪽 부분 배열 요소를 temp에 복사
        // rightIndex가 high 보다 크지 않은 경우 (즉, 아직 오른쪽 부분 배열에 요소가 남아있는 경우) 계속 진행
        "cmp %[ri], %[high]\n\t"
        "bgt finish_merge\n\t"           // leftIndex가 mid보다 크면 루프를 종료하고 merge 작업을 마무리
        "ldr r6, [%[a], %[ri], LSL #2]\n\t"  // 업데이트된 a[rightIndex]의 값을 r6에 로드
        "str r6, [%[temp], %[ti], LSL #2]\n\t" // r6 레지스터의 값을 temp[tempIndex]에 저장
        "add %[ri], %[ri], #1\n\t"       // rightIndex 증가
        "add %[ti], %[ti], #1\n\t"       // tempIndex 증가
        "b check_right\n\t"                // 다시 check_right 레이블로 점프하여 남은 왼쪽 요소를 계속 복사

        "finish_merge:\n\t"
        // temp의 내용을 원래의 배열 a에 복사
        "mov %[ti], #0\n\t"              // tempIndex를 0으로 초기화
        "copy_back_loop:\n\t"
        "cmp %[ti], %[n]\n\t"            // tempIndex와 n을 비교
        "bge end_copy_back\n\t"          // tempIndex가 n 이상이면 모든 요소를 복사했음을 의미하고, 복사 루프를 종료
        "ldr r5, [%[temp], %[ti], LSL #2]\n\t" // temp[tempIndex]에서 요소를 r5 레지스터로 로드
        "str r5, [%[a], %[low], LSL #2]\n\t" // r5 레지스터의 값을 a[low + tempIndex]에 저장
        "add %[low], %[low], #1\n\t"     // low 값을 증가시키며 배열 인덱스를 조정 (여기서 실수했음)
        "add %[ti], %[ti], #1\n\t"       // tempIndex 증가
        "b copy_back_loop\n\t"           // 다시 copy_back_loop 레이블로 점프하여 나머지 요소를 계속 복사

        "end_copy_back:\n\t"
        :
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high),
        [li] "r" (leftIndex), [ri] "r" (rightIndex), [ti] "r" (tempIndex), [temp] "r" (temp), [n] "r" (n)
        : "r5", "r6", "cc", "memory"
        );

    free(temp); // 임시 배열 해제
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
