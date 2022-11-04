#pragma once
#include <cstdint>
class pawn_tt_entry
{
public:
	static const int VAL_UNKNOWN = 200000;
	pawn_tt_entry();
	pawn_tt_entry(uint64_t set_key, int set_eg_score, int set_mg_score);
	~pawn_tt_entry();
	int eg_score = VAL_UNKNOWN;
	int mg_score = VAL_UNKNOWN;
	uint64_t key = 0ULL;	
};