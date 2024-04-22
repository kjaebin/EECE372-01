void merge_ASM(int* a, int low, int mid, int high) {
    asm (
        "ldr r1, %[a]\n"             // Base address of the array
        "add r2, r1, %[low], lsl #2\n"   // Address of a[low]
        "add r3, r1, %[mid], lsl #2\n"   // Address of a[mid]
        "add r4, r1, %[mid], lsl #2\n"   // Address of a[mid+1]
        "add r4, r4, #4\n"
        "add r5, r1, %[high], lsl #2\n"  // Address of a[high]
        "add r5, r5, #4\n"
        "mov r6, r2\n"                // Initialize write-back pointer r6 to start at r2

        "merge_loop:\n"
        "cmp r2, r3\n"
        "bhi process_right\n"
        "cmp r4, r5\n"
        "bhi process_left\n"
        "ldr r7, [r2]\n"
        "ldr r8, [r4]\n"
        "cmp r7, r8\n"
        "ble store_left\n"
        "str r8, [r6]\n"
        "add r4, r4, #4\n"
        "b update_index\n"
        "store_left:\n"
        "str r7, [r6]\n"
        "add r2, r2, #4\n"
        "update_index:\n"
        "add r6, r6, #4\n"
        "b merge_loop\n"

        "process_right:\n"
        "cmp r4, r5\n"
        "bhi end\n"
        "ldr r7, [r4]\n"
        "str r7, [r6]\n"
        "add r4, r4, #4\n"
        "add r6, r6, #4\n"
        "b process_right\n"

        "process_left:\n"
        "cmp r2, r3\n"
        "bhi end\n"
        "ldr r7, [r2]\n"
        "str r7, [r6]\n"
        "add r2, r2, #4\n"
        "add r6, r6, #4\n"
        "b process_left\n"

        "end:\n"
        :
        : [a] "r" (a), [low] "r" (low), [mid] "r" (mid), [high] "r" (high)
        : "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "cc", "memory"
        );
}
