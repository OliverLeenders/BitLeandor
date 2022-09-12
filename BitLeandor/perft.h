#pragma once
#include <cstdint>
#include "bitboard.h"
#include "movelist.h"
#include "movegen.h"
class perft
{
public:
	static uint64_t run_perft(bitboard* b, int depth);
	static uint64_t run_perft_console(bitboard* b, int depth);
private:

};

