OBJS = $(wildcard src/*)

CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic

LDFLAGS =

BIN = anagram

all : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BIN) $(LDFLAGS)
