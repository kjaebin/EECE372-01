#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function prototypes
void mergesort_C(int* a, int low, int high);
void merge_C(int* a, int low, int mid, int high);

// Helper function to print arrays
void printArray(int* a, int size);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    // Handle user input
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

    if (!data) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Initialize and shuffle the array
    for (int i = 0; i < n; i++) {
        data[i] = i + 1;  // Sequential filling
    }
    for (int i = 0; i < n - 1; i++) {
        int r = rand() % (n - i) + i;
        int temp = data[i];
        data[i] = data[r];
        data[r] = temp;
    }

    // Print data before sorting
    if (n <= 200) {
        printf("Before sort: [ ");
        for (int i = 0; i < n; i++) {
            printf("%d ", data[i]);
        }
        printf("]\n");
    }

    // Time measurement for C sorting
    clock_t begin = clock();
    mergesort_C(data, 0, n - 1);
    clock_t end = clock();
    float elapsed_c = (float)(end - begin) / CLOCKS_PER_SEC;

    // Print data after C sorting
    if (n <= 200) {
        printf("After sort (C): [ ");
        for (int i = 0; i < n; i++) {
            printf("%d ", data[i]);
        }
        printf("]\n");
    }

    // Print execution time for C
    printf("Execution Time (C): %.6f [sec]\n", elapsed_c);

    free(data);
    return 0;
}

void mergesort_C(int* a, int low, int high) {
    if (low < high) {
        int mid = low + (high - low) / 2;
        mergesort_C(a, low, mid);
        mergesort_C(a, mid + 1, high);
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

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}
