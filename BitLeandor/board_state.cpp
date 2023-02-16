#include "board_state.h"

board_state::board_state(uint64_t set_z_hash, uint64_t set_ep_target_square, bit_move set_last_move, int set_fifty_mr_counter,  char set_castling_rights)
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
