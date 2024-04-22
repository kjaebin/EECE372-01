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
    if (!temp) {
        perror("Failed to allocate memory for temporary array");
        return;
    }

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

#include <stdlib.h>

void merge_ASM(int* a, int low, int mid, int high) {
    int n1 = mid - low + 1;
    int n2 = high - mid;
    int* L = malloc(n1 * sizeof(int)); // Left subarray
    int* R = malloc(n2 * sizeof(int)); // Right subarray

    // Copy data to temporary subarrays L[] and R[]
    for (int i = 0; i < n1; i++) L[i] = a[low + i];
    for (int i = 0; i < n2; i++) R[i] = a[mid + 1 + i];

    int i = 0; // Initial index of first subarray
    int j = 0; // Initial index of second subarray
    int k = low; // Initial index to be sorted

    // Implement the merging in assembly
    asm volatile (
        // Setup initial indices for L and R arrays
        "mov r4, %0\n"         // r4 = i
        "mov r5, %1\n"         // r5 = j
        "mov r6, %2\n"         // r6 = k
        "mov r7, %3\n"         // r7 = L
        "mov r8, %4\n"         // r8 = R

        "merge_loop:\n"
        "cmp r4, %5\n"         // Compare i with n1
        "bge merge_done_right\n"
        "cmp r5, %6\n"         // Compare j with n2
        "bge merge_done_left\n"

        // Load values from L and R
        "ldr r9, [r7, r4, lsl #2]\n"   // Load L[i]
        "ldr r10, [r8, r5, lsl #2]\n"  // Load R[j]

        // Compare and store smallest in a[k]
        "cmp r9, r10\n"
        "bgt store_right\n"
        "str r9, [%7, r6, lsl #2]\n"   // Store L[i] at a[k]
        "add r4, r4, #1\n"             // Increment i
        "b update_k\n"
        "store_right:\n"
        "str r10, [%7, r6, lsl #2]\n"  // Store R[j] at a[k]
        "add r5, r5, #1\n"             // Increment j

        "update_k:\n"
        "add r6, r6, #1\n"             // Increment k
        "b merge_loop\n"

        "merge_done_right:\n"
        // Finish merging remaining R elements if any
        "cmp r5, %6\n"
        "bge merge_cleanup\n"
        "ldr r10, [r8, r5, lsl #2]\n"
        "str r10, [%7, r6, lsl #2]\n"
        "add r5, r5, #1\n"
        "add r6, r6, #1\n"
        "b merge_done_right\n"

        "merge_done_left:\n"
        // Finish merging remaining L elements if any
        "cmp r4, %5\n"
        "bge merge_cleanup\n"
        "ldr r9, [r7, r4, lsl #2]\n"
        "str r9, [%7, r6, lsl #2]\n"
        "add r4, r4, #1\n"
        "add r6, r6, #1\n"
        "b merge_done_left\n"

        "merge_cleanup:\n"
        :
        : "r" (i), "r" (j), "r" (k), "r" (L), "r" (R), "r" (n1), "r" (n2), "r" (a)
        : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "cc", "memory"
    );

    free(L);
    free(R);
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
