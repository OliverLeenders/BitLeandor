#include "board_state.h"

board_state::board_state(uint64_t set_z_hash, uint64_t set_p_hash, uint64_t set_ep_target_square,
                         bit_move set_last_move, int16_t set_PST_score_MG, int16_t set_PST_score_EG,
                         uint16_t set_fifty_mr_counter, char set_castling_rights) {
    // clang-format off
	z_hash 					 = set_z_hash;
	p_hash 					 = set_p_hash;
	castling_rights 		 = set_castling_rights;
	en_passant_target_square = set_ep_target_square;
	last_move 				 = set_last_move;
	PST_score_MG 			 = set_PST_score_MG;
	PST_score_EG			 = set_PST_score_EG;
	fifty_move_counter 		 = set_fifty_mr_counter;
    // clang-format on
}

board_state::~board_state() {}
