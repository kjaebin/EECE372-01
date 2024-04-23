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
