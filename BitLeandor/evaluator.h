#pragma once

#include "bitboard.h"

class evaluator
{
public:
	static int eval_material(bitboard* b);
	static void init_tables();
	
	enum {
		MIDGAME = 0,
		ENDGAME = 1
	};
	const static int piece_values[2][6];
	const static int game_phase_values[6];
	static int piece_square_tables[2][2][6][64];
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
private:

};
