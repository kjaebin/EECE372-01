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
    if (low < high) {
        int mid = low + (high - low) / 2;

        // Recursively sort both halves (still in C for recursion management)
        mergesort_ASM(a, low, mid);
        mergesort_ASM(a, mid + 1, high);

        // Use inline assembly for merging; already implemented in merge_ASM
        merge_ASM(a, low, mid, high);
    }
}

void merge_ASM(int* a, int low, int mid, int high) {
    // Temporary array to hold merged results
    int* temp = (int*)malloc((high - low + 1) * sizeof(int));
    if (!temp) {
        perror("Memory allocation failed");
        return;
    }

    // Initialize pointers and indexes
    int i = low, j = mid + 1, k = 0;

    // Inline assembly block to perform the merging operation
    asm(
        "1: \n" // Loop label
        "cmp %[i], %[mid] \n" // Compare i and mid
        "bgt 2f \n" // If i > mid, jump to 2
        "cmp %[j], %[high] \n" // Compare j and high
        "bgt 3f \n" // If j > high, jump to 3

        // Load elements from both halves
        "ldr r6, [%[a], %[i], lsl #2] \n" // Load a[i] into r6
        "ldr r7, [%[a], %[j], lsl #2] \n" // Load a[j] into r7

        // Compare elements
        "cmp r6, r7 \n"
        "ble 4f \n" // If a[i] <= a[j], go to 4

        // Store a[j] in temp[k], increment j and k
        "str r7, [%[temp], %[k], lsl #2] \n"
        "add %[j], %[j], #1 \n"
        "b 5f \n"

        "4: \n" // Store a[i] in temp[k], increment i and k
        "str r6, [%[temp], %[k], lsl #2] \n"
        "add %[i], %[i], #1 \n"

        "5: \n" // Increment k and loop back
        "add %[k], %[k], #1 \n"
        "b 1b \n"

        "2: \n" // Handle remaining elements from right half
        "cmp %[j], %[high] \n"
        "bgt 3f \n"
        "ldr r6, [%[a], %[j], lsl #2] \n"
        "str r6, [%[temp], %[k], lsl #2] \n"
        "add %[j], %[j], #1 \n"
        "add %[k], %[k], #1 \n"
        "b 2b \n"

        "3: \n" // Handle remaining elements from left half
        : [temp] "=r"(temp), [i]"=r"(i), [j]"=r"(j), [k]"=r"(k) // Output
        : [a] "r"(a), [mid] "r"(mid), [low] "r"(low), [high] "r"(high) // Input
        : "r6", "r7", "memory" // Clobbers
    );

    // Copy from temp back to array
    for (i = low, k = 0; i <= high; i++, k++) {
        a[i] = temp[k];
    }

    free(temp);

    return;
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
