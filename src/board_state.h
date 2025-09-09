#pragma once
#include "bit_move.h"
#include <cstdint>

class board_state {
  public:
    board_state(uint64_t set_z_hash, uint64_t set_p_hash, uint64_t set_ep_target_square,
                bit_move set_last_move, int16_t set_PST_score_MG, int16_t set_PST_score_EG,
                uint16_t set_fifty_mr_counter, char set_castling_rights, int set_game_phase);
    ~board_state();
    uint64_t z_hash = 0ULL;
    uint64_t p_hash = 0ULL;
    uint64_t en_passant_target_square = 0ULL;
    bit_move last_move = bit_move();
    int16_t PST_score_MG = 0;
    int16_t PST_score_EG = 0;
    uint16_t fifty_move_counter = 0;
    int game_phase = 0;
    char castling_rights = 0;
};