#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//user defined function
void mergesort_C(<inputs>);
void merge_C(<inputs>);
void mergesort_ASM(<inputs>);
void merge_ASM(<inputs>);


int main(int argc, char* argv[]) {
	srand(time(NULL));

	// user input
	// variable initialization

	// print data before sorting

	// measuring time, recommended to use clock_gettime()
	mergesort_C(<inputs>);
	// measuring time

	// measuring time
	mergesort_ASM(<inputs>);
	// measuring time

	// print data after sorting
	// print run time

	return 0;
}

void mergesort_ASM(<inputs>) {
	//local variable

	// recursion
	mergesort_ASM(<inputs>);
	mergesort_ASM(<inputs>);

	//function call
	merge_ASM(<inputs>);

	return;
}

void merge(<inputs>) {
	//local variable

	asm(
		// inline assembly
	);
	return;
}