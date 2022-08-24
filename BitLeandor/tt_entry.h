#pragma once
#include "bit_move.h"

class tt_entry
{
public:
	tt_entry();
	tt_entry(int set_score, int set_depth, int set_type, bit_move set_move);
	~tt_entry();
	bit_move hash_move = bit_move();
	int score = 0;
	uint8_t depth = 0;
	uint8_t type = 0;
	enum
	{
		EXACT = 0,
		LOWER_BOUND = 1,
		UPPER_BOUND = 2
	};
};

