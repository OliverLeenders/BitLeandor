#pragma once

#include "bitboard.h"
#include "transposition_table.h"
#include "pawn_tt_entry.h"
#include "pawn_transposition_table.h"

class evaluator
{
public:
	static int eval(bitboard* b);
	static int eval_pawn_structure(bitboard* b, uint8_t phase);
	static void init_tables();
	
	enum {
		MIDGAME = 0,
		ENDGAME = 1
	};
	const static int piece_values[2][12];
	const static uint8_t game_phase_values[6];
	const static uint8_t mobility_divisors[6];
	
	// distance arrays
	static int manhattan_distance[64][64];
	const static int center_manhattan_distance[64];
	
	// pawn spans
	static uint64_t front_spans[2][64];
	static uint64_t attack_front_spans[2][64];
	
	// piece square tables (PST)
	const static int pawn_pst_mg[64];
	const static int pawn_pst_eg[64];
	const static int knight_pst_mg[64];
	const static int knight_pst_eg[64];
	const static int bishop_pst_mg[64];
	const static int bishop_pst_eg[64];
	const static int rook_pst_mg[64];
	const static int rook_pst_eg[64];
	const static int queen_pst_mg[64];
	const static int queen_pst_eg[64];
	const static int king_pst_mg[64];
	const static int king_pst_eg[64];
	const static int passed_pawn_bonus_pst_mg[64];
	const static int passed_pawn_bonus_pst_eg[64];
	
	static int pp_bonus[2][2][64]; // [color][phase][square]
	const static int double_pawn_penalty[2][2][8]; // [color][phase][file]

	// master PST
	static int piece_square_tables[2][12][64];
	
	// pawn structure transposition table
	static pawn_transposition_table pawn_tt;
};
