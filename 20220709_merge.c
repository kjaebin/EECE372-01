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
    int i, j, k;
    int n1 = mid - low + 1;
    int n2 = high - mid;

    int L[n1], H[n2];

    for (i = 0; i < n1; i++)
        L[i] = a[low + i];
    for (j = 0; j < n2; j++)
        H[j] = a[mid + 1 + j];

    i = 0;
    j = 0;
    k = low;

    while (i < n1 && j < n2) {
        if (L[i] <= H[j]) {
            a[k] = L[i];
            i++;
        }
        else {
            a[k] = H[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        a[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        a[k] = H[j];
        j++;
        k++;
    }
}

void mergesort_ASM(int* a, int low, int high) {
    asm(
        "cmp %[low], %[high] \n"
        "bge end_function \n"  // End of the function if low >= high

        // Calculate middle index
        "mov r4, %[high] \n"
        "add r4, r4, %[low] \n"
        "lsr r4, r4, #1 \n"  // r4 = (low + high) >> 1

        // Recursive call to sort the first half
        "push {lr} \n"
        "sub sp, sp, #12 \n"  // Make space for parameters on stack
        "str r4, [sp, #8] \n"  // Save r4 (mid) for second recursive call
        "mov %[high], r4 \n"
        "bl mergesort_ASM \n"
        "add sp, sp, #12 \n"
        "pop {lr} \n"

        // Recursive call to sort the second half
        "push {lr} \n"
        "sub sp, sp, #8 \n"
        "ldr r4, [sp, #16] \n"  // Recover saved mid
        "add r4, r4, #1 \n"
        "mov %[low], r4 \n"
        "bl mergesort_ASM \n"
        "add sp, sp, #8 \n"
        "pop {lr} \n"

        // Merge the two halves
        "ldr r3, [sp, #16] \n"  // Recover original mid
        "push {r0-r3} \n"
        "bl merge_ASM \n"
        "pop {r0-r3} \n"

        "end_function:\n"  // Label end_function as the end of function
        :
        : [a] "r" (a), [low] "r" (low), [high] "r" (high)
        : "r3", "r4", "cc", "memory"
        );
}


void merge_ASM(int* a, int low, int mid, int high) {
    asm(
        "mov r9, %[a] \n"  // Base address of the array
        "mov r8, %[low] \n"  // Start index of the left subarray
        "mov r7, %[mid] \n"  // End index of the left subarray
        "add r6, %[mid], #1 \n"  // Start index of the right subarray
        "mov r5, %[high] \n"  // End index of the right subarray
        "mov r10, r8 \n"  // Temporary index for the merged array

        "merge_loop: \n"  // Start of merge loop
        "cmp r8, r7 \n"
        "bgt process_right \n"  // If left index > mid, process right subarray
        "cmp r6, r5 \n"
        "bgt process_left \n"  // If right index > high, process left subarray

        "ldr r2, [r9, r8, lsl #2] \n"
        "ldr r3, [r9, r6, lsl #2] \n"
        "cmp r2, r3 \n"
        "ble store_left \n"
        "str r3, [r9, r10, lsl #2] \n"
        "add r6, r6, #1 \n"
        "b update_index \n"  // Jump forward to update_index

        "store_left: \n"
        "str r2, [r9, r10, lsl #2] \n"
        "add r8, r8, #1 \n"

        "update_index: \n"  // Update index and loop back
        "add r10, r10, #1 \n"
        "b merge_loop \n"  // Jump back to merge_loop

        "process_right: \n"  // Handling remaining elements from right subarray
        "ldr r2, [r9, r6, lsl #2] \n"
        "str r2, [r9, r10, lsl #2] \n"
        "add r6, r6, #1 \n"
        "b update_index \n"  // Go back to update_index

        "process_left: \n"  // Handling remaining elements from left subarray
        "ldr r2, [r9, r8, lsl #2] \n"
        "str r2, [r9, r10, lsl #2] \n"
        "add r8, r8, #1 \n"
        "b update_index \n"  // Go back to update_index
        :
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high)
        : "r2", "r3", "r5", "r6", "r7", "r8", "r9", "r10", "cc", "memory"
        );
}



void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
