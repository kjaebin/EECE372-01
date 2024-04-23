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
    printf("Execution Time   (C): %.f [sec]\n", elapsed_c);
    printf("Execution Time (ASM): %.f [sec]\n", elapsed_asm);

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
    if (low < high) {
        int mid = low + (high - low) / 2;

        mergesort_ASM(a, low, mid);

        mergesort_ASM(a, mid + 1, high);

        merge_ASM(a, low, mid, high);
    }
}

void merge_ASM(int* a, int low, int mid, int high) {

    // C언어에서 로컬 변수 선언
    int leftIndex, rightIndex, tempIndex;
    int* temp;
    int n;

    asm(
        "mov r1, %[li]\n"          // leftIndex
        "mov r2, %[ri]\n"          // rightIndex
        "mov r3, %[ti]\n"          // tempIndex
        "mov r4, %[a]\n"           // 배열 a의 주소
        "mov r5, %[temp]\n"        // temp 배열의 주소

        "loop_1:\n"                // Merge the two sorted halves into a temporary array
        "cmp r1, %[mid]\n"         // leftIndex와 mid 비교
        "bgt left_done\n"          // leftIndex > mid이면 left_done으로 점프
        "cmp r2, %[high]\n"        // rightIndex와 high 비교
        "bgt right_done\n"         // rightIndex > high이면 right_done으로 점프

        "ldr r6, [r4, r1, LSL #2]\n" // a[leftIndex]
        "ldr r7, [r4, r2, LSL #2]\n" // a[rightIndex]
        "cmp r6, r7\n"
        "ble copy_left\n"          // r6 <= r7 이면 copy_left로 점프
        "b copy_right\n"

        "copy_left:\n"
        "str r6, [r5, r3, LSL #2]\n" // temp[tempIndex] = a[leftIndex]
        "add r1, r1, #1\n"           // leftIndex++
        "b increment_temp\n"

        "copy_right:\n"
        "str r7, [r5, r3, LSL #2]\n" // temp[tempIndex] = a[rightIndex]
        "add r2, r2, #1\n"           // rightIndex++

        "increment_temp:\n"
        "add r3, r3, #1\n"           // tempIndex++
        "b loop_1\n"

        "left_done:\n"
        "right_done:\n"
        // Copy any remaining elements from the left half
        "check_left:\n"
        "cmp %[li], %[mid]\n"       // Compare leftIndex with mid
        "bgt end_left\n"            // If leftIndex > mid, jump to end_left
        "ldr r6, [%[a], %[li], LSL #2]\n"  // Load the value at a[leftIndex]
        "str r6, [%[temp], %[ti], LSL #2]\n" // Store it in temp[tempIndex]
        "add %[li], %[li], #1\n"    // Increment leftIndex
        "add %[ti], %[ti], #1\n"    // Increment tempIndex
        "b check_left\n"            // Loop back to check_left

        "end_left:\n"
        // Copy any remaining elements from the right half
        "check_right:\n"
        "cmp %[ri], %[high]\n"      // Compare rightIndex with high
        "bgt end_right\n"           // If rightIndex > high, jump to end_right
        "ldr r6, [%[a], %[ri], LSL #2]\n" // Load the value at a[rightIndex]
        "str r6, [%[temp], %[ti], LSL #2]\n" // Store it in temp[tempIndex]
        "add %[ri], %[ri], #1\n"    // Increment rightIndex
        "add %[ti], %[ti], #1\n"    // Increment tempIndex
        "b check_right\n"           // Loop back to check_right

        "end_right:\n"
        // Copy the sorted elements back into the original array
        "mov r7, #0\n"              // Initialize counter i = 0
        "copy_back:\n"
        "cmp r7, %[n]\n"            // Compare i with n
        "bge finish_copy\n"         // If i >= n, finish copying
        "ldr r6, [%[temp], r7, LSL #2]\n" // Load the value from temp[i]
        "str r6, [%[a], %[low], LSL #2]\n" // Store it in a[low + i]
        "add r7, r7, #1\n"          // Increment i
        "add %[low], %[low], #4\n"  // Move the base pointer of a to the next element
        "b copy_back\n"             // Loop back to copy_back

        "finish_copy:\n"
        :
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high),
        [li] "r" (leftIndex), [ri] "r" (rightIndex), [ti] "r" (tempIndex), [temp] "r" (temp),
        [n] "r" (n)
        : "r6", "r7", "cc", "memory"
        );
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
