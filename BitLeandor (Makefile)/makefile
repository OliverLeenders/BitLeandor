CC=g++
CFLAGS=-Wall -Wpedantic -Werror -g -O3 -o engine

engine: main.cpp board.cpp piece.cpp search.cpp movegen.cpp evaluator.cpp 
	$(CC) $(CFLAGS) main.cpp board.cpp piece.cpp search.cpp movegen.cpp evaluator.cpp