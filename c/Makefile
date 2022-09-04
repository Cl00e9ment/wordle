CC=gcc
CFLAGS=-O3 -fopenmp -msse4.2 -Wall -pedantic

.PHONY: all clean

all: wordle

clean:
	rm -f wordle

wordle: wordle.c utils.c utils.h
	$(CC) $(CFLAGS) -o $@ wordle.c utils.c
