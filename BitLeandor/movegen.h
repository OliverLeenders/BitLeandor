#pragma once

#include <list>
#include "bit_move.h"
#include "bitboard_util.h"
#include "bitboards.h"
#include "movelist.h"

class movegen
{
public:
	movegen();
	~movegen();

	static void generate_pawn_qmoves(bitboard b, movelist* l);
    static void generate_pawn_cmoves(bitboard b, movelist* l);
	static void generate_knight_qmoves(bitboard b, movelist* l);
	static void generate_knight_cmoves(bitboard b, movelist* l);
};

