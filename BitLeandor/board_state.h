#pragma once
#include <cstdint>
#include "bit_move.h"

class board_state {
	public:
	board_state(uint64_t set_z_hash, char set_castling_rights, char set_ep_target_square, int set_fifty_mr_counter, bit_move set_last_move);
	~board_state();
	uint64_t z_hash = 0ULL;
	char castling_rights = 0;
	char en_passant_target_square = 0;
	int fifty_move_counter = 0;
	bit_move last_move = bit_move();
};