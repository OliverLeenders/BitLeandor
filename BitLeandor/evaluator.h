#pragma once

#include "bitboard.h"

class evaluator
{
public:
	static int eval_material(bitboard* b);
	enum {
		MIDGAME = 0,
		ENDGAME = 1
	};
	const static int piece_values[2][6];
	const static int game_phase_values[6];
private:

};

evaluator::evaluator()
{
}

evaluator::~evaluator()
{
}