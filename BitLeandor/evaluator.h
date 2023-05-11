/**
 * @file evaluator.h
 * @author Oliver Leenders (oliver.leenders@gmx.net)
 * @brief Contains all information and functions to evaluate chess positions.
 * @version 0.1
 * @date 2023-05-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include "bitboard.h"
#include "transposition_table.h"
#include "pawn_tt_entry.h"
#include "pawn_transposition_table.h"

enum {
	MIDGAME = 0,
	ENDGAME = 1,
	NUM_PHASES = 2
};

/**
 * @brief Contains all functions relating to evaluating a position.
 */
class evaluator
{
public:
	static int eval(bitboard* b);
	static void eval_pawn_structure(bitboard* b, int* mg_score, int* eg_score);
	static bool is_draw(bitboard* b);

	static int tuner_eval(bitboard* b);
	static void init_tables();

	const static int piece_values[NUM_PHASES][12];
	static int tuner_piece_values[NUM_PHASES][6];
	const static uint8_t game_phase_values[6];
	const static uint8_t game_phase_sum;
	const static float mobility_factors[6];

	// distance arrays
	static int manhattan_distance[NUM_SQUARES][NUM_SQUARES];
	const static int center_manhattan_distance[64];

	// pawn spans
	static uint64_t front_spans[NUM_COLORS][NUM_SQUARES];
	static uint64_t attack_front_spans[NUM_COLORS][NUM_SQUARES];

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

	static int pp_bonus[2][NUM_PHASES][NUM_SQUARES]; // [color][phase][square]
	static int double_pawn_penalty[NUM_PHASES][8]; // [phase][file]

	/**
	 * @brief Master PST. Combines all piece square tables into a single multi-dimensional array.
	 * indexed by [PHASE][PIECE][SQUARE]
	*/
	static int piece_square_tables[NUM_PHASES][12][NUM_SQUARES];

	// pawn structure transposition table
	static pawn_transposition_table pawn_tt;

	static const uint64_t pawn_shields[NUM_COLORS][NUM_SIDES];
	static const uint64_t safe_king_positions[NUM_COLORS][NUM_SIDES];
	static int pawn_tt_hits;
	static int pawn_tt_misses;

};
