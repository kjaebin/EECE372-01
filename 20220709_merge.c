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
    int leftIndex, rightIndex, tempIndex;
    int* temp;
    int n;

    asm(
        "mov r1, %[low]\n"         // Initialize leftIndex with low
        "mov r2, %[mid]\n"         // Initialize rightIndex with mid + 1
        "add r2, r2, #1\n"
        "mov r3, #0\n"             // Initialize tempIndex with 0
        "mov r4, %[a]\n"           // Pointer to the array 'a'
        "mov r5, %[temp]\n"        // Pointer to the temporary array 'temp'
        // Merge two sorted halves into a temporary array
        "loop_1:\n"
        "cmp r1, %[mid]\n"
        "bgt left_done\n"
        "cmp r2, %[high]\n"
        "bgt right_done\n"
        "ldr r6, [r4, r1, LSL #2]\n"
        "ldr r7, [r4, r2, LSL #2]\n"
        "cmp r6, r7\n"
        "ble copy_left\n"
        "b copy_right\n"
        "copy_left:\n"
        "str r6, [r5, r3, LSL #2]\n"
        "add r1, r1, #1\n"
        "b increment_temp\n"
        "copy_right:\n"
        "str r7, [r5, r3, LSL #2]\n"
        "add r2, r2, #1\n"
        "increment_temp:\n"
        "add r3, r3, #1\n"
        "b loop_1\n"
        "left_done:\n"
        "right_done:\n"
        // Remaining elements copy
        "check_left:\n"
        "cmp r1, %[mid]\n"
        "bgt end_left\n"
        "ldr r6, [r4, r1, LSL #2]\n"
        "str r6, [r5, r3, LSL #2]\n"
        "add r1, r1, #1\n"
        "add r3, r3, #1\n"
        "b check_left\n"
        "end_left:\n"
        "check_right:\n"
        "cmp r2, %[high]\n"
        "bgt end_right\n"
        "ldr r6, [r4, r2, LSL #2]\n"
        "str r6, [r5, r3, LSL #2]\n"
        "add r2, r2, #1\n"
        "add r3, r3, #1\n"
        "b check_right\n"
        "end_right:\n"
        // Copy back to the original array
        "mov r7, #0\n"             // Initialize index for copy back
        "copy_back:\n"
        "cmp r7, %[n]\n"
        "bge finish_copy\n"
        "ldr r6, [r5, r7, LSL #2]\n"
        "str r6, [r4, r7, LSL #2]\n"
        "add r7, r7, #1\n"
        "b copy_back\n"
        "finish_copy:\n"
        :
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high),
          [li] "r" (leftIndex), [ri] "r" (rightIndex), [ti] "r" (tempIndex), [temp] "r" (temp),
          [n] "r" (n)
        : "r1", "r2", "r3", "r4", "r5", "r6", "r7", "cc", "memory"
    );
}


void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
