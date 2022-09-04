#pragma once

typedef struct {
	char     raw[6];     // rusty
	uint32_t letters; // --R--T-S--U-------Y-------
} Word;

void sort_words(Word *A, size_t n);
