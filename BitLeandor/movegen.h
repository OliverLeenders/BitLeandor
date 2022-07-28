#pragma once

#include <list>
#include "bit_move.h"
#include "bitboard_util.h"
#include "bitboard.h"
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

	static void generate_bishop_qmoves(bitboard b, movelist* l);
	static void generate_bishop_cmoves(bitboard b, movelist* l);

	static void generate_rook_qmoves(bitboard b, movelist* l);
	static void generate_rook_cmoves(bitboard b, movelist* l);

	static void generate_king_qmoves(bitboard b, movelist* l);
	static void generate_king_cmoves(bitboard b, movelist* l);
};

