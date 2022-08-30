#pragma once

#include "tt_entry.h"
#include "utility.h"

class transposition_table {
public:
	static const int VAL_UNKNOWN = 200000;
	transposition_table(int size);
	transposition_table();
	int size = 16777216;
	static void init_keys();
	tt_entry* table = new tt_entry[16777216];
	static uint64_t piece_keys[12][64];
	static uint64_t castling_keys[4];
	static uint64_t en_passant_keys[8];
	static uint64_t side_key;
	void set(uint64_t key, int depth, int eval, int flag, bit_move move);
	int probe(uint64_t key, int depth, int ply, int alpha, int beta, bit_move* m);
	int probe_qsearch(uint64_t key, int ply, int alpha, int beta, bit_move* m);
	~transposition_table();
};