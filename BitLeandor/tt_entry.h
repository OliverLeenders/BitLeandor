#pragma once
#include "bit_move.h"

class tt_entry
{
public:
	static const int VAL_UNKNOWN = 200000;
	tt_entry();
	tt_entry(uint64_t key, int set_score, int set_depth, int set_type, bit_move set_move);
	~tt_entry();
	bit_move hash_move = bit_move();
	int score = VAL_UNKNOWN;
	uint64_t key = 0;
	uint8_t depth = 0;
	uint8_t type = NONE;
	enum
	{
		EXACT = 0,
		LOWER_BOUND = 1,
		UPPER_BOUND = 2,
		NONE = 3
	};
};

