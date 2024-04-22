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
    double elapsed_c = (double)(end1 - begin1) / CLOCKS_PER_SEC;

    // Sorting with Assembly implementation
    begin2 = clock();
    mergesort_ASM(data_asm, 0, n - 1);
    end2 = clock();
    double elapsed_asm = (double)(end2 - begin2) / CLOCKS_PER_SEC;

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
    printf("Execution Time   (C): %.6lf [sec]\n", elapsed_c);
    printf("Execution Time (ASM): %.6lf [sec]\n", elapsed_asm);

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
    printf("mergesort_ASM called with low=%d, high=%d\n", low, high);
    if (low < high) {
        int mid = low + (high - low) / 2;
        printf("Dividing: low=%d, mid=%d, high=%d\n", low, mid, high);

        mergesort_ASM(a, low, mid);
        mergesort_ASM(a, mid + 1, high);

        merge_ASM(a, low, mid, high);
    }
}

void merge_ASM(int* a, int low, int mid, int high) {
    // Allocate memory for the temporary array
    int* temp = (int*)malloc((high - low + 1) * sizeof(int));
    if (!temp) {
        perror("Memory allocation failed");
        return;
    }

    // Print the initial state of the array segment being merged
    printf("Initial array (a) state for merge:\n");
    for (int idx = low; idx <= high; idx++) {
        printf("%d ", a[idx]);
    }
    printf("\n");

    int i = low, j = mid + 1, k = 0;

    // Inline assembly for merging two sorted halves
    asm volatile (
        "1:\n"                               // Label for the top of the merge loop
        "cmp %1, %3\n"                       // Compare i with mid
        "bgt 2f\n"                           // If i > mid, jump to process the right half
        "cmp %2, %5\n"                       // Compare j with high
        "bgt 3f\n"                           // If j > high, jump to process the left half

        "ldr r6, [%0, %1, lsl #2]\n"         // Load a[i] into r6
        "ldr r7, [%0, %2, lsl #2]\n"         // Load a[j] into r7
        "cmp r6, r7\n"                       // Compare the elements a[i] and a[j]
        "ble 4f\n"                           // If a[i] <= a[j], go to store a[i] in temp

        "str r7, [%4, %6, lsl #2]\n"         // Store a[j] in temp[k]
        "add %2, %2, #1\n"                   // Increment j
        "b 5f\n"                             // Jump to increment k

        "4:\n"                               // Label to store a[i] in temp
        "str r6, [%4, %6, lsl #2]\n"         // Store a[i] in temp[k]
        "add %1, %1, #1\n"                   // Increment i

        "5:\n"                               // Label to increment k and loop back
        "add %6, %6, #1\n"                   // Increment k
        "b 1b\n"                             // Jump back to the top of the loop

        "2:\n"                               // Label to process remaining right half elements
        "ldr r6, [%0, %2, lsl #2]\n"         // Load a[j] into r6
        "str r6, [%4, %6, lsl #2]\n"         // Store a[j] in temp[k]
        "add %2, %2, #1\n"                   // Increment j
        "add %6, %6, #1\n"                   // Increment k
        "b 2b\n"                             // Continue processing the right half

        "3:\n"                               // Label to process remaining left half elements
        "ldr r6, [%0, %1, lsl #2]\n"         // Load a[i] into r6
        "str r6, [%4, %6, lsl #2]\n"         // Store a[i] in temp[k]
        "add %1, %1, #1\n"                   // Increment i
        "add %6, %6, #1\n"                   // Increment k
        : "+r" (i), "+r" (j), "+r" (k)       // Output operands
        : "r" (mid), "r" (high), "r" (temp), "r" (a) // Input operands
        : "r6", "r7", "cc", "memory"         // Clobbers
    );

    // Copy the merged temporary array back to the original array segment
    for (i = low, k = 0; i <= high; i++, k++) {
        a[i] = temp[k];
    }

    printf("Merged array segment:\n");
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
