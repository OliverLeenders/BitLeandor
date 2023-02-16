#pragma once
#include <cstdint>
#include "bit_move.h"

class board_state {
	public:
	board_state(uint64_t set_z_hash, uint64_t set_ep_target_square, bit_move set_last_move, int set_fifty_mr_counter, char set_castling_rights);
	~board_state();
	uint64_t z_hash = 0ULL;
	uint64_t en_passant_target_square = 0ULL;
	int fifty_move_counter = 0;
	bit_move last_move = bit_move();
	char castling_rights = 0;
};