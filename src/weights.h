/**
 * @file weights.h
 * @author Oliver Leenders (oliver.leenders@gmx.net)
 * @brief class containing all relevant evaluation weights
 * @version 0.1
 * @date 2023-05-15
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <cmath>

#include "constants.h"

enum : uint8_t { MIDGAME = 0, ENDGAME = 1, NUM_PHASES = 2 };

class weights {
  public:
    // pawn spans

    static const uint64_t pawn_shields[NUM_COLORS][NUM_SIDES];
    static const uint64_t safe_king_positions[NUM_COLORS][NUM_SIDES];

    const static int piece_values[NUM_PHASES][12];
    static int tuner_piece_values[NUM_PHASES][6];
    const static float mobility_factors[6];

    // piece square tables (PST)
    const static int pawn_pst_mg[NUM_SQUARES];
    const static int pawn_pst_eg[NUM_SQUARES];
    const static int knight_pst_mg[NUM_SQUARES];
    const static int knight_pst_eg[NUM_SQUARES];
    const static int bishop_pst_mg[NUM_SQUARES];
    const static int bishop_pst_eg[NUM_SQUARES];
    const static int rook_pst_mg[NUM_SQUARES];
    const static int rook_pst_eg[NUM_SQUARES];
    const static int queen_pst_mg[NUM_SQUARES];
    const static int queen_pst_eg[NUM_SQUARES];
    const static int king_pst_mg[NUM_SQUARES];
    const static int king_pst_eg[NUM_SQUARES];
    const static int passed_pawn_bonus_pst_mg[NUM_SQUARES];
    const static int passed_pawn_bonus_pst_eg[NUM_SQUARES];

    static int pawn_shield_penalty_ks;
    static int pawn_shield_penalty_qs;
    static int isolated_pawn_penalties[NUM_PHASES][8];

    static int pp_bonus[NUM_COLORS][NUM_PHASES][NUM_SQUARES]; // [color][phase][square]
    static int double_pawn_penalty[NUM_PHASES][8];            // [phase][file]
    static int pawn_island_penalty[NUM_PHASES];   // [phase]

    /**
     * @brief Master PST. Combines all piece square tables into a single multi-dimensional array.
     * indexed by [PHASE][PIECE][SQUARE]
     */
    static int piece_square_tables[NUM_PHASES][12][NUM_SQUARES];

    const static uint8_t game_phase_values[6];
    const static uint8_t game_phase_sum;

    static void init_tables();
};
