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
    printf("Execution Time   (C): %f [sec]\n", elapsed_c);
    printf("Execution Time (ASM): %f [sec]\n", elapsed_asm);

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
    asm(
        // Initialize registers r1, r2 only if they are not set (can use a flag or check if r1, r2 are zero)
        "cmp r1, #0\n\t"
        "cmpeq r2, #0\n\t"
        "moveq r1, %[l]\n\t"               // Set r4 to low if not already set
        "moveq r2, %[h]\n\t"               // Set r5 to high if not already set
        
        // �� ������ �����Ͽ� low�� high�� ��
        "mov r0, %[a]\n\t"                 // �迭 ������ a�� r0�� ����
        "cmp r1, r2\n\t"               // ��: low >= high
        "bge end_mergesort\n\t"            // ���� low >= high �̸�, ����� ���̽� ���̽��� ���������Ƿ� end_mergesort�� �б�

        // �߰� ���� ���
        "sub r3, r2, r1\n\t"           // r3 = high - low, �迭 ���� ���
        "lsr r3, r3, #1\n\t"               // r3 = (high - low) / 2, ���������� �� ��Ʈ ����Ʈ�Ͽ� 2�� ����
        "add r3, r1, r3\n\t"             // r3 = low + (high - low) / 2, �߰� �ε��� ���

        // ù ��° ��� ȣ��: ���� �κ� �迭 ����
        "push {r0-r3, lr}\n\t"             // r0���� r3������ �������Ϳ� ��ũ ��������(lr)�� ���ÿ� ����
        "mov r2, r3\n\t"                   // �߰� �ε��� mid�� r2�� ����
        "bl mergesort_ASM\n\t"             // mergesort_ASM �Լ� ȣ��
        "pop {r0-r3, lr}\n\t"              // ���ÿ��� r0���� r3������ �������Ϳ� ��ũ ��������(lr)�� ����

        // �� ��° ��� ȣ��: ������ �κ� �迭 ����
        "push {r0-r3, lr}\n\t"             // r0���� r3������ �������Ϳ� ��ũ ��������(lr)�� ���ÿ� ����
        "mov r1, r3\n\t"                   // �߰� �ε��� mid�� r1�� �ٽ� ����
        "add r1, r1, #1\n\t"               // r1 = mid + 1, ������ �κ� �迭�� ���� �ε��� ����
        "mov r2, %[h]\n\t"                 // ���� �ε��� high�� r2�� ����
        "bl mergesort_ASM\n\t"             // mergesort_ASM �Լ� ȣ��
        "pop {r0-r3, lr}\n\t"              // ���ÿ��� r0���� r3������ �������Ϳ� ��ũ ��������(lr)�� ����

        // ���� ȣ��: ���ĵ� �� �κ� �迭 ����
        "push {r0-r3, lr}\n\t"             // r0���� r3������ �������Ϳ� ��ũ ��������(lr)�� ���ÿ� ����
        "mov r4, r2\n\t"                   // ���� �ε��� high�� temp�� r4�� ��� ����
        "mov r2, r3\n\t"                   // �߰� �ε��� mid�� r2�� ����
        "mov r3, r4\n\t"                   // ���� �ε��� high�� r3�� ����
        "bl merge_ASM\n\t"                 // merge_ASM �Լ� ȣ��
        "pop {r0-r3, lr}\n\t"              // ���ÿ��� r0���� r3������ �������Ϳ� ��ũ ��������(lr)�� ����

        "end_mergesort:\n\t"               // ����� ���̽� ���̽� �� �Լ� ���� ���� ���̺�
        :
        : [a] "r" (a), [l] "r" (low), [h] "r" (high)  // �Է�: �迭 ������, ���� �ε���, ���� �ε���
        : "r0", "r1", "r2", "r3", "r4", "lr", "memory", "cc"  // clobbered: ���� �������Ϳ� �޸�, ���� �ڵ�
        );
}

void merge_ASM(int* a, int low, int mid, int high) {
    int leftIndex = low, rightIndex = mid + 1, tempIndex = 0;
    int* temp = (int*)malloc((high - low + 1) * sizeof(int)); // �ӽ� �迭�� ���� �޸� �Ҵ�
    int n = high - low + 1; // Number of elements to merge

    asm(
        // �ʱ� �������� ����
        "mov %[li], %[low]\n\t"            // leftIndex�� low ������ �ʱ�ȭ
        "mov %[ri], %[mid]\n\t"            // rightIndex�� mid ������ �ʱ�ȭ
        "add %[ri], %[ri], #1\n\t"         // rightIndex�� mid+1�� �����Ͽ� ������ �κ� �迭�� ���������� ����
        "mov %[ti], #0\n\t"                // tempIndex�� 0���� �ʱ�ȭ

        // ���� ���� ����
        "loop_merge:\n\t"
        "cmp %[li], %[mid]\n\t"            // leftIndex�� mid ��
        "bgt left_done\n\t"                 // leftIndex�� mid���� ũ�� left �κ� �迭�� �������� �ǹ�, left_done���� ����
        "cmp %[ri], %[high]\n\t"           // rightIndex�� high ��
        "bgt right_done\n\t"                // rightIndex�� high���� ũ�� right �κ� �迭�� �������� �ǹ�, right_done���� ����

        // �� �κ� �迭�� ���� ��Ҹ� �ε�
        "ldr r5, [%[a], %[li], LSL #2]\n\t"  // a[leftIndex]�� ���� r5�� �ε�
        "ldr r6, [%[a], %[ri], LSL #2]\n\t"  // a[rightIndex]�� ���� r6�� �ε�

        // �� �� temp�� ����
        "cmp r5, r6\n\t"
        "bgt copy_right\n\t"                // r5 > r6 �̸� ������ ��Ҹ� temp�� ����

        "copy_left:\n\t"
        "str r5, [%[temp], %[ti], LSL #2]\n\t"  // temp[tempIndex]�� a[leftIndex]�� ����
        "add %[li], %[li], #1\n\t"         // leftIndex ����
        "b increment_temp\n\t"             // tempIndex ������ ����

        "copy_right:\n\t"
        "str r6, [%[temp], %[ti], LSL #2]\n\t"  // temp[tempIndex]�� a[rightIndex]�� ����
        "add %[ri], %[ri], #1\n\t"         // rightIndex ����

        "increment_temp:\n\t"
        "add %[ti], %[ti], #1\n\t"         // tempIndex ����
        "b loop_merge\n\t"                 // ���� ������ ���ư���

        "left_done:\n\t"
        "right_done:\n\t"
        "check_left:\n\t"
        // ���� ���� �κ� �迭 ��Ҹ� temp�� ����
        // leftIndex�� mid ���� ũ�� ���� ��� (��, ���� ���� �κ� �迭�� ��Ұ� �����ִ� ���) ��� ����
        "cmp %[li], %[mid]\n\t"
        "bgt end_left\n\t"           // rightIndex�� high���� ũ�� ������ �����ϰ� merge �۾��� ������
        "ldr r5, [%[a], %[li], LSL #2]\n\t" // ������Ʈ�� a[leftIndex]�� ���� r5�� �ε�
        "str r5, [%[temp], %[ti], LSL #2]\n\t" // r5 ���������� ���� temp[tempIndex]�� ����
        "add %[li], %[li], #1\n\t"       // leftIndex ����
        "add %[ti], %[ti], #1\n\t"       // tempIndex ����
        "b check_left\n\t"                 // �ٽ� check_left ���̺�� �����Ͽ� ���� ������ ��Ҹ� ��� ����

        "end_left:\n\t"
        "check_right:\n\t"
        // ���� ������ �κ� �迭 ��Ҹ� temp�� ����
        // rightIndex�� high ���� ũ�� ���� ��� (��, ���� ������ �κ� �迭�� ��Ұ� �����ִ� ���) ��� ����
        "cmp %[ri], %[high]\n\t"
        "bgt finish_merge\n\t"           // leftIndex�� mid���� ũ�� ������ �����ϰ� merge �۾��� ������
        "ldr r6, [%[a], %[ri], LSL #2]\n\t"  // ������Ʈ�� a[rightIndex]�� ���� r6�� �ε�
        "str r6, [%[temp], %[ti], LSL #2]\n\t" // r6 ���������� ���� temp[tempIndex]�� ����
        "add %[ri], %[ri], #1\n\t"       // rightIndex ����
        "add %[ti], %[ti], #1\n\t"       // tempIndex ����
        "b check_right\n\t"                // �ٽ� check_right ���̺�� �����Ͽ� ���� ���� ��Ҹ� ��� ����

        "finish_merge:\n\t"
        // temp�� ������ ������ �迭 a�� ����
        "mov %[ti], #0\n\t"              // tempIndex�� 0���� �ʱ�ȭ
        "copy_back_loop:\n\t"
        "cmp %[ti], %[n]\n\t"            // tempIndex�� n�� ��
        "bge end_copy_back\n\t"          // tempIndex�� n �̻��̸� ��� ��Ҹ� ���������� �ǹ��ϰ�, ���� ������ ����
        "ldr r5, [%[temp], %[ti], LSL #2]\n\t" // temp[tempIndex]���� ��Ҹ� r5 �������ͷ� �ε�
        "str r5, [%[a], %[low], LSL #2]\n\t" // r5 ���������� ���� a[low + tempIndex]�� ����
        "add %[low], %[low], #1\n\t"     // low ���� ������Ű�� �迭 �ε����� ���� (���⼭ �Ǽ�����)
        "add %[ti], %[ti], #1\n\t"       // tempIndex ����
        "b copy_back_loop\n\t"           // �ٽ� copy_back_loop ���̺�� �����Ͽ� ������ ��Ҹ� ��� ����

        "end_copy_back:\n\t"
        :
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high),
        [li] "r" (leftIndex), [ri] "r" (rightIndex), [ti] "r" (tempIndex), [temp] "r" (temp), [n] "r" (n)
        : "r5", "r6", "cc", "memory"
        );

    free(temp); // �ӽ� �迭 ����
}

void printArray(int* a, int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}