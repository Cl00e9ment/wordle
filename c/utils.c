/// quicksort implementation inspired from Wikipedia (https://en.wikipedia.org/wiki/Merge_sort#Bottom-up_implementation)

#include <stdint.h>
#include <stdlib.h>

#include "utils.h"

const uint32_t JQ = 1<<27 | 1<<26;

//  Left run is A[iLeft :iRight-1].
// Right run is A[iRight:iEnd-1  ].
void bottom_up_merge(const Word *A, size_t iLeft, size_t iRight, size_t iEnd, Word* B) {
	size_t i = iLeft;
	size_t j = iRight;

	// While there are elements in the left or right runs...
	for (size_t k = iLeft; k < iEnd; ++k) {
		// If left run head exists and is <= existing right run head.
		if (i < iRight && (j >= iEnd || (A[i].letters ^ JQ) <= (A[j].letters ^ JQ))) {
			B[k] = A[i];
			i = i + 1;
		} else {
			B[k] = A[j];
			j = j + 1;
		}
	}
}

uint32_t min(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

// array A[] has the items to sort; array B[] is a work array
void bottom_up_merge_sort(Word *A, Word *B, size_t n) {
	// Each 1-element run in A is already "sorted".
	// Make successively longer sorted runs of length 2, 4, 8, 16... until the whole array is sorted.
	for (size_t width = 1; width < n; width = 2 * width) {
		// Array A is full of runs of length width.
		for (size_t i = 0; i < n; i = i + 2 * width) {
			// Merge two runs: A[i:i+width-1] and A[i+width:i+2*width-1] to B[]
			// or copy A[i:n-1] to B[] ( if (i+width >= n) )
			bottom_up_merge(A, i, min(i + width, n), min(i + 2 * width, n), B);
		}
		// Now work array B is full of runs of length 2*width.
		// Copy array B to array A for the next iteration.
		// A more efficient implementation would swap the roles of A and B.
		Word *tmp = A;
		A = B;
		B = tmp;
		// Now array A is full of runs of length 2*width.
	}
}

void sort_words(Word *A, size_t n) {
	Word B[n];
	bottom_up_merge_sort(A, B, n);
}
