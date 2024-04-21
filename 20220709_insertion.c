#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// user defined function
void insertion_C(int a[], int n);
void insertion_ASM(int a[], int n);

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
    for (int i = 0; i < n; i++) {
        data[i] = rand() % 100;  // Random values between 0 and 99
        data_asm[i] = data[i];   // Copy to assembly data array
    }

	// print data before sorting
    printf("Before sort: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");

    clock_t begin1, end1;
    clock_t begin2, end2;

	// measuring time, recommended to use clock_gettime()
    // measure time with C implementation
    begin1 = clock();
    insertion_C(data, n);
    end1 = clock();

    // measure time with Assembly implementation
    begin2 = clock();
    insertion_ASM(data_asm, n);
    end2 = clock();

    double elapsed_c = (double)(end1 - begin1) / CLOCKS_PER_SEC;
    double elapsed_asm = (double)(end2 - begin2) / CLOCKS_PER_SEC;

	// print data after sorting
    printf("After sort (C): ");
    for (int i = 0; i < n; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");

    printf("After sort (ASM): ");
    for (int i = 0; i < n; i++) {
        printf("%d ", data_asm[i]);
    }
    printf("\n");

	// print run time
    printf("Execution Time (C): %.9lf [sec]\n", elapsed_c);
    printf("Execution Time (ASM): %.9lf [sec]\n", elapsed_asm);

    free(data);
    free(data_asm);

	return 0;
}

void insertion_C(int a[], int n) {
	int i, j, key;
	for (i = 1; i < n; i++) {
		key = a[i];
		j = i - 1;
		while (j >= 0 && a[j] > key) {
			a[j + 1] = a[j];
			j--;
		}
		a[j + 1] = key;
	}
}

void insertion_ASM(int a[], int n) {
    asm(
        "mov r1, #1\n" // i = 1
        "loop_i:\n\t"
        "cmp r1, %[n]\n\t"
        "bge end_i\n\t"
        "ldr r3, [%[a], r1, LSL #2]\n\t" // key = a[i]
        "sub r2, r1, #1\n" // j = i - 1
        "loop_j:\n\t"
        "cmp r2, #0\n\t"
        "blt update_a\n\t"
        "ldr r4, [%[a], r2, LSL #2]\n\t" // temp = a[j]
        "cmp r4, r3\n\t"
        "ble update_a\n\t"
        "add r5, r2, #1\n\t"
        "str r4, [%[a], r5, LSL #2]\n\t" // a[j + 1] = a[j]
        "sub r2, r2, #1\n\t"
        "b loop_j\n"
        "update_a:\n\t"
        "add r2, r2, #1\n\t"
        "str r3, [%[a], r2, LSL #2]\n\t" // a[j + 1] = key
        "add r1, r1, #1\n\t"
        "b loop_i\n"
        "end_i:\n\t"
        :
        : [n] "r"(n), [a] "r"(a)
        : 
        "r1", "r2", "r3", "r4", "r5"
     );
}
