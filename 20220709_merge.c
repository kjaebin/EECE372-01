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
    int leftIndex = low, rightIndex = mid + 1, tempIndex = 0;
    int* temp = (int*)malloc((high - low + 1) * sizeof(int)); // 임시 배열을 위한 메모리 할당
    int n = high - low + 1; // Number of elements to merge

    asm volatile (
        "mov %[li], %[low]\n"            // Initialize leftIndex to low
        "mov %[ri], %[mid]\n"            // Initialize rightIndex to mid
        "add %[ri], %[ri], #1\n"         // Set rightIndex to mid + 1
        "mov %[ti], #0\n"                // Initialize tempIndex to 0

        "loop_merge:\n"
        "cmp %[li], %[mid]\n"            // Compare leftIndex with mid
        "bgt process_right\n"            // If leftIndex > mid, process remaining right items
        "cmp %[ri], %[high]\n"           // Compare rightIndex with high
        "bgt process_left\n"             // If rightIndex > high, process remaining left items

        "ldr r5, [%[a], %[li], LSL #2]\n"// Load a[leftIndex]
        "ldr r6, [%[a], %[ri], LSL #2]\n"// Load a[rightIndex]
        "cmp r5, r6\n"
        "bgt copy_right\n"               // If left item > right item, copy from right

        "copy_left:\n"
        "str r5, [%[temp], %[ti], LSL #2]\n"
        "add %[li], %[li], #1\n"
        "b increment_index\n"

        "copy_right:\n"
        "str r6, [%[temp], %[ti], LSL #2]\n"
        "add %[ri], %[ri], #1\n"

        "increment_index:\n"
        "add %[ti], %[ti], #1\n"
        "b loop_merge\n"

        "process_right:\n"
        "cmp %[ri], %[high]\n"
        "bgt finish_merge\n"
        "ldr r6, [%[a], %[ri], LSL #2]\n"
        "str r6, [%[temp], %[ti], LSL #2]\n"
        "add %[ri], %[ri], #1\n"
        "add %[ti], %[ti], #1\n"
        "b process_right\n"

        "process_left:\n"
        "cmp %[li], %[mid]\n"
        "bgt finish_merge\n"
        "ldr r5, [%[a], %[li], LSL #2]\n"
        "str r5, [%[temp], %[ti], LSL #2]\n"
        "add %[li], %[li], #1\n"
        "add %[ti], %[ti], #1\n"
        "b process_left\n"

        "finish_merge:\n"
        "mov %[ti], #0\n"
        "copy_back:\n"
        "cmp %[ti], %[n]\n"
        "bge end_copy_back\n"
        "ldr r5, [%[temp], %[ti], LSL #2]\n"
        "str r5, [%[a], %[low], LSL #2]\n"
        "add %[ti], %[ti], #1\n"
        "add %[low], %[low], #4\n"
        "b copy_back\n"

        "end_copy_back:\n"

        : // No output operands
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high),
          [li] "r" (leftIndex), [ri] "r" (rightIndex), [ti] "r" (tempIndex), [temp] "r" (temp), [n] "r" (n)
        : "r5", "r6", "cc", "memory"
    );

    free(temp); // Free temporary array
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
