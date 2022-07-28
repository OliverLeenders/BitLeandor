#include "board_state.h"

board_state::board_state(uint64_t set_z_hash, char set_castling_rights, char set_ep_target_square, int set_fifty_mr_counter,  bit_move set_last_move)
{
	z_hash = set_z_hash;
	castling_rights = set_castling_rights;
	en_passant_target_square = set_ep_target_square;
	last_move = set_last_move;
	fifty_move_counter = set_fifty_mr_counter;
}

board_state::~board_state()
{
}
