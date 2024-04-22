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

void merge_ASM(int* a, int low, int mid, int high) {
    asm (
        "mov %[low], %[low]\n"  // Example to pass variable from C to asm
        "mov %[mid], %[mid]\n"
        "mov %[high], %[high]\n"
        "mov r1, %[a]\n"  // Base address of the array

        // Initialize pointers for array indices
        "add r2, r1, %[low], lsl #2\n"   // Pointer to low
        "add r3, r1, %[mid], lsl #2\n"   // Pointer to mid
        "add r4, r1, %[mid], lsl #2\n"   // Pointer to mid + 1
        "add r3, r3, #4\n"               // Correcting mid pointer to one element right
        "add r5, r1, %[high], lsl #2\n"  // Pointer to high
        "add r5, r5, #4\n"               // Correcting high pointer to one element right

        "1:\n"  // Label for the start of loop
        "cmp r2, r3\n"  // Compare low pointer with mid
        "bge 2f\n"      // If low >= mid, jump to label 2
        "cmp r4, r5\n"  // Compare mid + 1 with high
        "bge 3f\n"      // If mid + 1 >= high, jump to label 3

        // Load values from left and right subarrays, compare and store
        "ldr r6, [r2]\n"
        "ldr r7, [r4]\n"
        "cmp r6, r7\n"
        "ble 4f\n"
        "str r7, [r1]\n"
        "add r4, r4, #4\n"
        "b 5f\n"
        "4:\n"
        "str r6, [r1]\n"
        "add r2, r2, #4\n"
        "5:\n"
        "add r1, r1, #4\n"
        "b 1b\n"

        "2:\n"  // Process remaining elements from the right subarray
        "cmp r4, r5\n"
        "bge 6f\n"
        "ldr r6, [r4]\n"
        "str r6, [r1]\n"
        "add r4, r4, #4\n"
        "add r1, r1, #4\n"
        "b 2b\n"

        "3:\n"  // Process remaining elements from the left subarray
        "cmp r2, r3\n"
        "bge 6f\n"
        "ldr r6, [r2]\n"
        "str r6, [r1]\n"
        "add r2, r2, #4\n"
        "add r1, r1, #4\n"
        "b 3b\n"

        "6:\n"  // End of function
        :
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high)
        : "r1", "r2", "r3", "r4", "r5", "r6", "r7", "cc", "memory"  // Clobbers
        );
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
