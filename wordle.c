// ZapOKill: "now do it in python and c"
//   Anders: "Someone should do this in Rust"

#include <nmmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"

#define WORDS_BUFFER_SIZE 16000
#define CLOCK_DIFF_MILLIS(end, start) ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000)

//      25                        0
//       ABCDEFGHIJKLMNOPQRSTUVWXYZ
// rusty -----------------RSTU---Y-

// Alister: "Would there be any advantage in having your word list sorted by letter frequency?"
//  Morgan: "i wonder if it could be improved by using a non-alphabetical letter bitset order"

// https://en.wikipedia.org/wiki/Letter_frequency
// An analysis of entries in the Concise Oxford dictionary, ignoring frequency of word use

//      25                        0
//       EARIOTNSLCUDPMHGBFYWKVXZJQ
// rusty --R--T-S--U-------Y-------

const uint32_t EARIOTNSLCUDPMHGBFYWKVXZJQ[27] = {
	0,
	1 << 24, // A
	1 << 9,  // B
	1 << 16, // C
	1 << 14, // D
	1 << 25, // E
	1 << 8,  // F
	1 << 10, // G
	1 << 11, // H
	1 << 22, // I
	1 << 27, // J (!)
	1 << 5,  // K
	1 << 17, // L
	1 << 12, // M
	1 << 19, // N
	1 << 21, // O
	1 << 13, // P
	1 << 26, // Q (!)
	1 << 23, // R
	1 << 18, // S
	1 << 20, // T
	1 << 15, // U
	1 << 4,  // V
	1 << 6,  // W
	1 << 3,  // X
	1 << 7,  // Y
	1 << 2,  // Z
};

uint32_t encode_word(char *raw) {
	uint32_t letters = 0;
	while (*raw) {
		letters |= EARIOTNSLCUDPMHGBFYWKVXZJQ[*raw & 31];
		++raw;
	}
	return letters;
}

Word words[WORDS_BUFFER_SIZE];
size_t words_len = 0;

void append_words(char *filename) {
	FILE *f = fopen(filename, "r");

	char raw[128];
	while (fgets(raw, 128, f)) {
		size_t raw_len = strlen(raw);
		if (raw[raw_len - 1] == '\n') {
			--raw_len;
			raw[raw_len] = '\0';
		}
		if (raw_len == 5) {
			uint32_t letters = encode_word(raw);
			if (_mm_popcnt_u64(letters) == 5) {
				words[words_len].letters = letters;
				memcpy(words[words_len].raw, raw, 6);
				++words_len;
			}
		}
	}

	fclose(f);
}

void write_solution(size_t i, size_t j, size_t k, size_t l, size_t m);
void write_anagrams(size_t index);

int main() {
	struct timespec t0;
	clock_gettime(CLOCK_REALTIME, &t0);

	// Uncomment the following 1 line to find 538 solutions:
	// appendWords("words_alpha.txt")
	// Uncomment the following 2 lines to find 10 solutions:
	append_words("wordle-nyt-answers-alphabetical.txt");
	append_words("wordle-nyt-allowed-guesses.txt");

	// xstrngr: "Your outer loop can be restricted to words with q or x   [q or j]
	//           when these run out, you can't have a solution with the remaining 24 letters"

	const uint32_t JQ = 1<<27 | 1<<26;
	sort_words(words, words_len);

	size_t jqSplit = 0;
	while (words[jqSplit].letters & JQ) {
		++jqSplit; // 284
	}

	//    0 jumby J-----------U--M--B-Y-------
	//    1 jumpy J-----------U-PM----Y-------
	//    2 judgy J-----------UD---G--Y-------
	//  ...
	//  189 hejra J-EAR-----------H-----------
	//  190 japer J-EAR---------P-------------
	//  191 rajes J-EAR----S------------------

	//  192 qophs -Q----O--S----P-H-----------
	//  193 quops -Q----O--S--U-P-------------
	//  194 quods -Q----O--S--UD--------------
	//  ...
	//  281 quena -QEA----N---U---------------
	//  282 quate -QEA---T----U---------------
	//  283 quare -QEAR-------U---------------

	//  284 vughy ------------U---HG--Y--V----
	//  285 bumpy ------------U-PM--B-Y-------
	//  286 whump ------------U-PMH----W------
	//  ...
	// 8307 terai --EARI-T--------------------
	// 8308 irate --EARI-T--------------------
	// 8309 retia --EARI-T--------------------

	uint32_t proxies[words_len];
	proxies[0] = words[0].letters;                    // J-----------U--M--B-Y-------
	size_t indices[words_len];
	indices[0] = 0;
	size_t proxies_len = 1;
	for (size_t i = 1; i < words_len; ++i) {
		if (words[i].letters != words[i-1].letters) {
			proxies[proxies_len] = words[i].letters;  // --EARI-T--------------------
			indices[proxies_len] = i;                 // indices[5175] = 8307
			++proxies_len;                            // 5176
		}
	}

	struct timespec t1;
	clock_gettime(CLOCK_REALTIME, &t1);

	// skip[a][b]

	//   Michael: "changing skip from a two-dimensional array to a one-dimensional array"
	// Pengochan: "one dimension could reduce memory accesses for the cost of a multiplication"

	// skip[a*WIDTH + b]

	size_t Y = proxies_len + 1;
	uint16_t *skip = malloc(proxies_len * Y * sizeof(uint16_t));
	uint16_t first[proxies_len];

	// NotPrivate: "Wouldn't it be faster if you Multithread for the whole time?"

	// 0000000000 1111111111 2222222222 3333333333 4444444444 5555555555 6666666666 7777777777
	#pragma omp parallel for schedule(static)
	for (size_t i = 0; i < proxies_len; ++i) {
		size_t iY = i * Y;
		uint16_t next = proxies_len; // 5176
		skip[iY + proxies_len] = next;
		uint32_t A = proxies[i];
		for (ssize_t j = (ssize_t) proxies_len - 1; j >= (ssize_t) i; --j) {
			uint32_t B = proxies[j];
			if ((A & B) == 0) {
				next = j;
			}
			skip[iY+j] = next;
		}
		first[i] = skip[iY+i];
	}

	struct timespec t2;
	clock_gettime(CLOCK_REALTIME, &t2);

	uint32_t result_counter = 0;

	// 01234567 01234567 01234567 01234567 01234567 01234567 01234567 01234567 01234567 01234567
	#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < jqSplit; ++i) {
		uint32_t A = proxies[i];
		size_t iY = i * Y;

		for (size_t j = first[i]; j < proxies_len; j = skip[iY + j + 1]) {
			uint32_t B = proxies[j];
			uint32_t AB = A | B;
			uint32_t jY = j * Y;

			for (size_t k = first[j]; k < proxies_len; k = skip[jY + skip[iY+k+1]]) {
				uint32_t C = proxies[k];
				if ((AB & C) != 0) {
					continue;
				}
				uint32_t ABC = AB | C;
				uint32_t kY = k * Y;

				for (size_t l = first[k]; l < proxies_len; l = skip[kY + skip[jY + skip[iY + l + 1]]]) {
					uint32_t D = proxies[l];
					if ((ABC & D) != 0) {
						continue;
					}
					uint32_t ABCD = ABC | D;
					uint32_t lY = l * Y;

					for (size_t m = first[l]; m < proxies_len; m = skip[lY + skip[kY + skip[jY + skip[iY + m + 1]]]]) {
						uint32_t E = proxies[m];
						if ((ABCD & E) != 0) {
							continue;
						}

						uint32_t count;
						#pragma omp atomic capture
						count = result_counter++;

						printf("%3d. ", count);
						write_solution(indices[i], indices[j], indices[k], indices[l], indices[m]);
					}
				}
			}
		}
	}

	struct timespec t3;
	clock_gettime(CLOCK_REALTIME, &t3);

	free(skip);

	printf("%4ldms prepare words\n", CLOCK_DIFF_MILLIS(t1, t0));
	printf("%4ldms compute tables\n", CLOCK_DIFF_MILLIS(t2, t1));
	printf("%4ldms find solutions\n", CLOCK_DIFF_MILLIS(t3, t2));
	printf("%4ldms total\n", CLOCK_DIFF_MILLIS(t3, t0));

	return EXIT_SUCCESS;
}

void write_solution(size_t i, size_t j, size_t k, size_t l, size_t m) {
	write_anagrams(i);
	putchar(' ');
	write_anagrams(j);
	putchar(' ');
	write_anagrams(k);
	putchar(' ');
	write_anagrams(l);
	putchar(' ');
	write_anagrams(m);
	putchar('\n');
}

// 1042 cylix -----I----LC--------Y---X---
// 1043 xylic -----I----LC--------Y---X---
// 1044 flick -----I----LC-------F--K-----

void write_anagrams(size_t index) {
	printf("%s", words[index].raw);      // cylix

	uint32_t letters = words[index].letters;    // -----I----LC--------Y---X---

	for (index++; words[index].letters == letters; index++) {
		printf("/%s", words[index].raw); // xylic
	}
}

// ZapOKill: "now do it in python and c"
//   Anders: "Someone should do this in Rust"

// Alister: "Would there be any advantage in having your word list sorted by letter frequency?"
//  Morgan: "i wonder if it could be improved by using a non-alphabetical letter bitset order"

// xstrngr: "Your outer loop can be restricted to words with q or x   [q or j]
//           when these run out, you can't have a solution with the remaining 24 letters"

//   Michael: "changing skip from a two-dimensional array to a one-dimensional array"
// Pengochan: "one dimension could reduce memory accesses for the cost of a multiplication"

// NotPrivate: "Wouldn't it be faster if you Multithread for the whole time?"

