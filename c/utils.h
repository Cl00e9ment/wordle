#pragma once

typedef struct {
	char     raw[6];     // rusty
	uint32_t letters; // --R--T-S--U-------Y-------
} Word;

typedef struct {
	char   buffer[1024];
	size_t len;
} StringBuilder;

void sort_words(Word *A, size_t n);
