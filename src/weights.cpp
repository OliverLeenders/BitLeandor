/**
 * @file weights.cpp
 * @author Oliver Leenders (oliver.leenders@gmx.net)
 * @brief Contains the values of the weights
 * @version 0.1
 * @date 2023-05-15
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "weights.h"

const int weights::piece_values[NUM_PHASES][12] = {{117, 391, 435, 652, 1288, 0,        // white MG piece values
                                                    -117, -391, -435, -652, -1288, 0},  // black MG piece values
                                                   {108, 396, 418, 634, 1258, 0,        // white EG piece values
                                                    -108, -396, -418, -634, -1258, 0}}; // black EG piece values

int weights::tuner_piece_values[NUM_PHASES][6] = {{117, 391, 435, 652, 1288, 0}, {108, 396, 418, 634, 1258, 0}};

const uint64_t weights::pawn_shields[NUM_COLORS][2] = {{57344ULL, 1792ULL},
                                                       {63050394783186944ULL, 1970324836974592ULL}};
const uint64_t weights::safe_king_positions[NUM_COLORS][2] = {{224ULL, 7ULL},
                                                              {16140901064495857664ULL, 504403158265495552ULL}};

int weights::isolated_pawn_penalties[NUM_PHASES][8] = {{3, 4, 6, 7, 7, 6, 4, 3}, {6, 8, 12, 14, 14, 12, 8, 6}};

const uint8_t weights::game_phase_values[6] = {0,
                                               4,  // 16
                                               4,  // 16
                                               6,  // 24
                                               10, // 20
                                               0};
// total : 16 + 16 + 24 + 20 = 76
const uint8_t weights::game_phase_sum = 76;

const float weights::mobility_factors[6] = {2.5, 2.5, // bishop
                                            2.0,      // rook
                                            0.5,      // queen
                                            0.5, 1};

int weights::piece_square_tables[NUM_COLORS][12][NUM_SQUARES] = {{{0}}};
int weights::pp_bonus[2][2][64] = {{{0}}}; // [color][phase][square]

int weights::pawn_shield_penalty_ks = 13;
int weights::pawn_shield_penalty_qs = 28;

// clang-format on

void weights::init_tables() {
    for (int i = 0; i < 64; i++) {
        int i_mirror = (7 - i / 8) * 8 + i % 8;
        piece_square_tables[MIDGAME][WHITE_PAWN][i] = pawn_pst_mg[i_mirror];
        piece_square_tables[MIDGAME][BLACK_PAWN][i] = -pawn_pst_mg[i];
        piece_square_tables[ENDGAME][WHITE_PAWN][i] = pawn_pst_eg[i_mirror];
        piece_square_tables[ENDGAME][BLACK_PAWN][i] = -pawn_pst_eg[i];

        piece_square_tables[MIDGAME][WHITE_KNIGHT][i] = knight_pst_mg[i_mirror];
        piece_square_tables[MIDGAME][BLACK_KNIGHT][i] = -knight_pst_mg[i];
        piece_square_tables[ENDGAME][WHITE_KNIGHT][i] = knight_pst_eg[i_mirror];
        piece_square_tables[ENDGAME][BLACK_KNIGHT][i] = -knight_pst_eg[i];

        piece_square_tables[MIDGAME][WHITE_BISHOP][i] = bishop_pst_mg[i_mirror];
        piece_square_tables[MIDGAME][BLACK_BISHOP][i] = -bishop_pst_mg[i];
        piece_square_tables[ENDGAME][WHITE_BISHOP][i] = bishop_pst_eg[i_mirror];
        piece_square_tables[ENDGAME][BLACK_BISHOP][i] = -bishop_pst_eg[i];

        piece_square_tables[MIDGAME][WHITE_ROOK][i] = rook_pst_mg[i_mirror];
        piece_square_tables[MIDGAME][BLACK_ROOK][i] = -rook_pst_mg[i];
        piece_square_tables[ENDGAME][WHITE_ROOK][i] = rook_pst_eg[i_mirror];
        piece_square_tables[ENDGAME][BLACK_ROOK][i] = -rook_pst_eg[i];

        piece_square_tables[MIDGAME][WHITE_QUEEN][i] = queen_pst_mg[i_mirror];
        piece_square_tables[MIDGAME][BLACK_QUEEN][i] = -queen_pst_mg[i];
        piece_square_tables[ENDGAME][WHITE_QUEEN][i] = queen_pst_eg[i_mirror];
        piece_square_tables[ENDGAME][BLACK_QUEEN][i] = -queen_pst_eg[i];

        piece_square_tables[MIDGAME][WHITE_KING][i] = king_pst_mg[i_mirror];
        piece_square_tables[MIDGAME][BLACK_KING][i] = -king_pst_mg[i];
        piece_square_tables[ENDGAME][WHITE_KING][i] = king_pst_eg[i_mirror];
        piece_square_tables[ENDGAME][BLACK_KING][i] = -king_pst_eg[i];

        pp_bonus[WHITE][MIDGAME][i] = passed_pawn_bonus_pst_mg[i_mirror];
        pp_bonus[WHITE][ENDGAME][i] = passed_pawn_bonus_pst_eg[i_mirror];
        pp_bonus[BLACK][MIDGAME][i] = -passed_pawn_bonus_pst_mg[i];
        pp_bonus[BLACK][ENDGAME][i] = -passed_pawn_bonus_pst_eg[i];
    }
}

// clang-format off
const int weights::pawn_pst_mg[64] = {
	 0,   0,   0,   0,   0,   0,    0,   0,
	 98, 134,  61,  95,  68, 126,  34, -11,
	 -6,   7,  26,  31,  65,  56,  25, -20,
	-14,  13,   6,  21,  23,  12,  17, -23,
	-27,  -2,  -5,  12,  17,   6,  10, -25,
	-26,  -4,  -4, -10,   3,   3,  33, -12,
	-35,  -1, -20, -23, -15,  24,  38, -22,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int weights::pawn_pst_eg[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	 94, 100,  85,  67,  56,  53,  82,  84,
	 32,  24,  13,   5,  -2,   4,  17,  17,
	 13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	  4,   7,  -6,   1,   0,  -5,  -1,  -8,
	 13,   8,   8,  10,  13,   0,   2,  -7,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int weights::passed_pawn_bonus_pst_eg[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	100, 120, 130, 130, 130, 130, 120, 100,
	 80,  90,  95, 100, 100,  95,  90,  80,
	 50,  60,  65,  70,  70,  65,  60,  50,
	 30,  40,  45,  50,  50,  45,  40,  30,
	 20,  30,  35,  40,  40,  35,  30,  20,
	 10,  20,  25,  30,  30,  25,  20,  10,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int weights::passed_pawn_bonus_pst_mg[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 10,  20,  30,  35,  35,  30,  20,  10,
	  5,  10,  15,  20,  20,  15,  10,   5,
	  0,   5,  10,  15,  15,  10,   5,   0,
	  0,   0,   5,  10,  10,   5,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int weights::knight_pst_mg[64] = {
	-167, -89, -34, -49,  61, -97, -15, -107,
	 -73, -41,  72,  36,  23,  62,   7,  -17,
	 -47,  60,  37,  65,  84, 129,  73,   44,
	  -9,  17,  19,  53,  37,  69,  18,   22,
	 -13,   4,  16,  13,  28,  19,  21,   -8,
	 -23,  -9,  12,  10,  19,  17,  25,  -16,
	 -29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23
};


const int weights::knight_pst_eg[64] = {
	-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,
};

const int weights::bishop_pst_mg[64] = {
	-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	 -4,   5,  19,  50,  37,  37,   7,  -2,
	 -6,  13,  13,  26,  34,  12,  10,   4,
	  0,  15,  15,  15,  14,  27,  18,  10,
	  4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,
};

const int weights::bishop_pst_eg[64] = {
	-14, -21, -11,  -8,  -7,  -9, -17, -24,
	 -8,  -4,   7, -12,  -3, -13,  -4, -14,
	  2,  -8,   0,  -1,  -2,   6,   0,   4,
	 -3,   9,  12,   9,  14,  10,   3,   2,
	 -6,   3,  13,  19,   7,  10,  -3,  -9,
	-12,  -3,   8,  10,  13,   3,  -7, -15,
	-14, -18,  -7,  -1,   4,  -9, -15, -27,
	-23,  -9, -23,  -5,  -9, -16,  -5, -17,
};

const int weights::rook_pst_mg[64] = {
	 32,  42,  32,  51,  63,   9,  31,  43,
	 27,  32,  58,  62,  80,  67,  26,  44,
	 -5,  19,  26,  36,  17,  45,  61,  16,
	-24, -11,   7,  26,  24,  35,  -8, -20,
	-36, -26, -12,  -1,   9,  -7,   6, -23,
	-45, -25, -16, -17,   3,   0,  -5, -33,
	-44, -16, -20,  -9,  -1,  11,  -6, -71,
	-19, -13,   1,  17,  16,   7, -37, -26,
};

const int weights::rook_pst_eg[64] = {
	 13,  10,  18,  15,  12,  12,   8,   5,
	 11,  13,  13,  11,  -3,   3,   8,   3,
	  7,   7,   7,   5,   4,  -3,  -5,  -3,
	  4,   3,  13,   1,   2,   1,  -1,   2,
	  3,   5,   8,   4,  -5,  -6,  -8, -11,
	 -4,   0,  -5,  -1,  -7, -12,  -8, -16,
	 -6,  -6,   0,   2,  -9,  -9, -11,  -3,
	 -9,   2,   3,  -1,  -5, -13,   4, -20,
};

const int weights::queen_pst_mg[64] = {
	-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	 -9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	 -1, -18,  -9,  10, -15, -25, -31, -50,
};

const int weights::queen_pst_eg[64] = {
	 -9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	  3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,
};

const int weights::king_pst_mg[64] = {
	-65,  23,  16, -15, -56, -34,   2,  13,
	 29,  -1, -20,  -7,  -8,  -4, -38, -29,
	 -9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	  1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14
};

const int weights::king_pst_eg[64] = {
	-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	 10,  17,  23,  15,  20,  45,  44,  13,
	 -8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,  4,   -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43
};



int weights::double_pawn_penalty[NUM_PHASES][8] = {
	{29, 22, 31, 10, 6, 35, 48, 24},
	{67, 35, 53, 16, 21, 48, 31, 49,}
};

int weights::pawn_island_penalty[NUM_PHASES] = { 10, 15 };


// 46 19 32 9 9 36 48 24
// 66 39 52 31 29 43 46 42

// clang format on

