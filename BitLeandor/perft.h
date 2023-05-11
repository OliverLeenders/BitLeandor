#pragma once
#include <cstdint>
#include <chrono>

#include "bitboard.h"
#include "movelist.h"
#include "movegen.h"
class perft
{
public:
	static uint64_t run_perft(bitboard* b, int depth);
	
	/**
	 * @brief Top-level perft test function. Prints perft info to the console
	 *
	 * @param b board representation
	 * @param depth depth to search
	 *
	 * @return number of leaf nodes
	 */
	static uint64_t run_perft_console(bitboard* b, int depth);
private:

};

