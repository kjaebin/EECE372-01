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

    // Handle user input
    if (argc < 2) {
        printf("Usage: %s <number of elements>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int* array = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        array[i] = rand() % 100; // Random numbers between 0 and 99
    }

    printf("Data before sorting:\n");
    printArray(array, n);

    // Time measurement setup
    clock_t begin1, end1;
    clock_t begin2, end2;

    // Sorting with C implementation
    begin1 = clock();
    mergesort_C(array, 0, n - 1);
    end1 = clock();
    double elapsed_c = (double)(end1 - begin1) / CLOCKS_PER_SEC;

    printf("Data after sorting (C):\n");
    printArray(array, n);
    printf("Execution Time (C): %f[s]\n", elapsed_c);

    // Sorting with Assembly implementation
    begin2 = clock();
    mergesort_ASM(array, 0, n - 1);
    end2 = clock();
    double elapsed_asm = (double)(end2 - begin2) / CLOCKS_PER_SEC;

    printf("Data after sorting (ASM):\n");
    printArray(array, n);
    printf("Execution Time (ASM): %f[s]\n", elapsed_asm);

    free(array);
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
    // Inline assembly for full mergesort including recursion and merging
    asm(
        // Check if low < high, if not, return
        "cmp %[low], %[high] \n"
        "bge .end \n"

        // Calculate middle index
        "add r3, %[low], %[high] \n"
        "asr r3, r3, #1 \n" // equivalent to (low + high) / 2

        // Recursive call on the first half
        "sub sp, sp, #32 \n"    // Allocate stack space for parameters + return address
        "str lr, [sp, #24] \n"  // Save return address
        "str r3, [sp, #20] \n"  // Save mid value for later use
        "str %[high], [sp, #16] \n" // Save high parameter
        "str %[low], [sp, #12] \n"  // Save low parameter
        "mov %[high], r3 \n"
        "bl .recursion \n"      // Branch to recursion label
        "ldr %[high], [sp, #16] \n" // Restore high parameter
        "ldr lr, [sp, #24] \n"       // Restore return address

        // Recursive call on the second half
        "ldr %[low], [sp, #12] \n"  // Restore low parameter
        "ldr r3, [sp, #20] \n"      // Restore mid value
        "add r3, r3, #1 \n"         // mid + 1
        "str r3, [sp, #20] \n"      // Save new mid value
        "mov %[low], r3 \n"
        "bl .recursion \n"
        "ldr lr, [sp, #24] \n"       // Restore return address

        // Call merge function
        "ldr %[low], [sp, #12] \n"  // Restore low parameter
        "ldr r3, [sp, #20] \n"      // Restore mid value
        "mov %[mid], r3 \n"
        "bl merge_ASM \n" // Merge function must be implemented properly in assembly

        ".recursion: \n"
        "bl mergesort_ASM \n" // Recursive call to self (mergesort_ASM)
        "b .return_from_call \n"

        ".return_from_call: \n"
        "add sp, sp, #32 \n"         // Deallocate stack space
        "bx lr \n"                   // Return from function using link register

        ".end: \n"
        :
        : [a] "r" (a), [low] "r" (low), [high] "r" (high), [mid] "r" (r3)
        : "r3", "memory", "cc"
        );
}

void merge_ASM(int* a, int low, int mid, int high) {
    // Temporary array to hold merged results
    int* temp = (int*)malloc((high - low + 1) * sizeof(int));
    if (!temp) {
        perror("Memory allocation failed");
        return;
    }

    printf("Initial array (a) state:\n");
    for (int idx = low; idx <= high; idx++) {
        printf("%d ", a[idx]);
    }
    printf("\n");

    // Initialize pointers and indexes
    int i = low, j = mid + 1, k = 0;

    // Inline assembly block to perform the merging operation
    asm volatile (
        "1:\n" // Loop label
        "cmp %1, %3\n" // Compare i and mid
        "bgt 2f\n" // If i > mid, jump to 2
        "cmp %2, %5\n" // Compare j and high
        "bgt 3f\n" // If j > high, jump to 3

        // Load elements from both halves
        "ldr r6, [%0, %1, lsl #2]\n" // Load a[i] into r6
        "ldr r7, [%0, %2, lsl #2]\n" // Load a[j] into r7

        // Compare elements
        "cmp r6, r7\n"
        "ble 4f\n" // If a[i] <= a[j], go to 4

        // Store a[j] in temp[k], increment j and k
        "str r7, [%4, %6, lsl #2]\n"
        "add %2, %2, #1\n"
        "b 5f\n"

        "4:\n" // Store a[i] in temp[k], increment i and k
        "str r6, [%4, %6, lsl #2]\n"
        "add %1, %1, #1\n"

        "5:\n" // Increment k and loop back
        "add %6, %6, #1\n"
        "b 1b\n"

        "2:\n" // Handle remaining elements from right half
        "ldr r6, [%0, %2, lsl #2]\n"
        "str r6, [%4, %6, lsl #2]\n"
        "add %2, %2, #1\n"
        "add %6, %6, #1\n"
        "b 2b\n"

        "3:\n" // Handle remaining elements from left half
        "ldr r6, [%0, %1, lsl #2]\n"
        "str r6, [%4, %6, lsl #2]\n"
        "add %1, %1, #1\n"
        "add %6, %6, #1\n"
        : "+r" (i), "+r" (j), "+r" (k)  // Output operands
        : "r" (mid), "r" (high), "r" (temp), "r" (a) // Input operands
        : "r6", "r7", "cc", "memory" // Clobbers
        );

    printf("Temporary array (temp) state:\n");
    for (int idx = 0; idx < (high - low + 1); idx++) {
        printf("%d ", temp[idx]);
    }
    printf("\n");

    // Copy from temp back to array
    for (i = low, k = 0; i <= high; i++, k++) {
        a[i] = temp[k];
    }

    printf("Final sorted array part:\n");
    for (int idx = low; idx <= high; idx++) {
        printf("%d ", a[idx]);
    }
    printf("\n");

    free(temp);
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
