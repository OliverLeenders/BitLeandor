#include "evaluator.h"

const int evaluator::piece_values[2][12] = {
	{ 100, 300, 330, 480, 900, 0 , -100, -300, -330, -480, -900, 0 },
	{ 120, 310, 340, 550, 1000, 0 , -120, -310, -340, -550, -1000, 0}
};

const uint8_t evaluator::game_phase_values[6] = {
	0,
	4, // 16
	4, // 16
	6, // 24
	10, // 20
	0
};

const uint8_t evaluator::mobility_divisors[6] = {
	1,
	1,
	// bishop
	2,
	// rook
	4,
	// queen
	6,
	1
};
// total : 16 + 16 + 24 + 20 = 76

int evaluator::piece_square_tables[2][12][64] = { {{0}} };
uint64_t evaluator::front_spans[2][64] = { {0} };
uint64_t evaluator::attack_front_spans[2][64] = { {0 } };


transposition_table evaluator::pawn_tt = transposition_table(4096 * 2);
int evaluator::manhattan_distance[64][64] = { {0} };

const int evaluator::center_manhattan_distance[64] = {
	6, 5, 4, 3, 3, 4, 5, 6,
	5, 4, 3, 2, 2, 3, 4, 5,
	4, 3, 2, 1, 1, 2, 3, 4,
	3, 2, 1, 0, 0, 1, 2, 3,
	3, 2, 1, 0, 0, 1, 2, 3,
	4, 3, 2, 1, 1, 2, 3, 4,
	5, 4, 3, 2, 2, 3, 4, 5,
	6, 5, 4, 3, 3, 4, 5, 6
};

const int evaluator::king_pst_eg[64] = {
	-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	 10,  17,  23,  15,  20,  45,  44,  13,
	 -8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,   4,  -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43
};


int evaluator::eval(bitboard* b)
{
	int midgame_score = 0;
	int endgame_score = 0;

	uint8_t phase = 0;

	uint8_t index = 0;
	uint64_t occ = b->occupancy[2];
	uint8_t piece = EMPTY;
	uint8_t wkp = BitScanForward64(b->bbs[KING][WHITE]);
	uint8_t bkp = BitScanForward64(b->bbs[KING][BLACK]);

	if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK])) {
		return 0;
	}
	else if ((b->bbs[PAWN][WHITE] | b->bbs[PAWN][BLACK]) == 0ULL) {

		//**************************************************************//
		//																//
		//  return 0 if endgame is drawn								//
		//																//
		//**************************************************************//

		// check KR-KR
		if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[ROOK][WHITE] | b->bbs[ROOK][BLACK])
			&& __popcnt64(b->bbs[ROOK][WHITE]) == 1
			&& __popcnt64(b->bbs[ROOK][BLACK]) == 1) {
			return 0;
		}
		// check KNN-K
		if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[KNIGHT][WHITE] | b->bbs[KNIGHT][BLACK])
			&& (
				(__popcnt64(b->bbs[KNIGHT][WHITE]) <= 2
					&& __popcnt64(b->bbs[KNIGHT][BLACK]) == 0)
				||
				(__popcnt64(b->bbs[KNIGHT][WHITE]) == 0
					&& __popcnt64(b->bbs[KNIGHT][BLACK]) <= 2))) {
			return 0;
		}
		// check KN-KB
		if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[KNIGHT][WHITE] | b->bbs[KNIGHT][BLACK] | b->bbs[BISHOP][WHITE] | b->bbs[BISHOP][BLACK])
			&& (
				(__popcnt64(b->bbs[KNIGHT][WHITE]) == 1
					&& __popcnt64(b->bbs[KNIGHT][BLACK]) == 0
					&& __popcnt64(b->bbs[BISHOP][BLACK]) == 1
					&& __popcnt64(b->bbs[BISHOP][WHITE]) == 0)
				||
				(__popcnt64(b->bbs[KNIGHT][WHITE]) == 0
					&& __popcnt64(b->bbs[KNIGHT][BLACK]) == 1
					&& __popcnt64(b->bbs[BISHOP][BLACK]) == 0
					&& __popcnt64(b->bbs[BISHOP][WHITE]) == 1))) {
			return 0;
		}
		// check KB-K
		if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[BISHOP][WHITE] | b->bbs[BISHOP][BLACK])
			&& (
				(__popcnt64(b->bbs[BISHOP][WHITE]) == 1
					&& __popcnt64(b->bbs[BISHOP][BLACK]) == 0)
				||
				(__popcnt64(b->bbs[BISHOP][WHITE]) == 0
					&& __popcnt64(b->bbs[BISHOP][BLACK]) == 1))) {
			return 0;
		}
	}

	//******************************************************************//
	// 																	//
	// evaluate material + piece square tables							//
	//																	//
	//******************************************************************//


	while (occ != 0ULL) {
		index = BitScanForward64(occ);
		piece = b->pieces[index];
		midgame_score += piece_values[MIDGAME][piece] + piece_square_tables[MIDGAME][piece][index];
		endgame_score += piece_values[ENDGAME][piece] + piece_square_tables[ENDGAME][piece][index];
		phase += game_phase_values[b->types[index]];
		occ &= occ - 1;
	}

	//******************************************************************//
	// 																	//
	// evaluate mobility [needs to be tweaked]							//
	//																	//
	//******************************************************************//

	uint64_t w_queens = b->bbs[QUEEN][WHITE];
	while (w_queens != 0ULL) {
		index = BitScanForward64(w_queens);
		midgame_score += (int)(__popcnt64(attacks::get_rook_attacks(index, occ)) + __popcnt64(attacks::get_bishop_attacks(index, occ))) / mobility_divisors[QUEEN];
		endgame_score += (int)(__popcnt64(attacks::get_rook_attacks(index, occ)) + __popcnt64(attacks::get_bishop_attacks(index, occ))) / mobility_divisors[QUEEN];
		w_queens &= w_queens - 1;
	}

	uint64_t b_queens = b->bbs[QUEEN][BLACK];
	while (b_queens != 0ULL) {
		index = BitScanForward64(b_queens);
		midgame_score -= (int)(__popcnt64(attacks::get_rook_attacks(index, occ)) + __popcnt64(attacks::get_bishop_attacks(index, occ))) / mobility_divisors[QUEEN];
		endgame_score -= (int)(__popcnt64(attacks::get_rook_attacks(index, occ)) + __popcnt64(attacks::get_bishop_attacks(index, occ))) / mobility_divisors[QUEEN];
		b_queens &= b_queens - 1;
	}

	uint64_t w_rooks = b->bbs[ROOK][WHITE];
	while (w_rooks != 0ULL) {
		index = BitScanForward64(w_rooks);
		midgame_score += (int)__popcnt64(attacks::get_rook_attacks(index, occ)) / mobility_divisors[ROOK];
		endgame_score += (int)__popcnt64(attacks::get_rook_attacks(index, occ)) / mobility_divisors[ROOK];
		w_rooks &= w_rooks - 1;
	}

	uint64_t b_rooks = b->bbs[ROOK][BLACK];
	while (b_rooks != 0ULL) {
		index = BitScanForward64(b_rooks);
		midgame_score -= (int)__popcnt64(attacks::get_rook_attacks(index, occ)) / mobility_divisors[ROOK];
		endgame_score -= (int)__popcnt64(attacks::get_rook_attacks(index, occ)) / mobility_divisors[ROOK];
		b_rooks &= b_rooks - 1;
	}

	uint64_t w_bishops = b->bbs[BISHOP][WHITE];
	while (w_bishops != 0ULL) {
		index = BitScanForward64(w_bishops);
		midgame_score += (int)__popcnt64(attacks::get_bishop_attacks(index, occ)) / mobility_divisors[BISHOP];
		endgame_score += (int)__popcnt64(attacks::get_bishop_attacks(index, occ)) / mobility_divisors[BISHOP];
		w_bishops &= w_bishops - 1;
	}

	uint64_t b_bishops = b->bbs[BISHOP][BLACK];
	while (b_bishops != 0ULL) {
		index = BitScanForward64(b_bishops);
		midgame_score -= (int)__popcnt64(attacks::get_bishop_attacks(index, occ)) / mobility_divisors[BISHOP];
		endgame_score -= (int)__popcnt64(attacks::get_bishop_attacks(index, occ)) / mobility_divisors[BISHOP];
		b_bishops &= b_bishops - 1;
	}

	//******************************************************************//
	// 																	//
	// special scoring for pawnless endgame	(found on CPW)				//
	//																	//
	//******************************************************************//

	int late_eg_score = 0;
	if (b->bbs[PAWN][WHITE] == 0ULL && b->bbs[PAWN][BLACK] == 0ULL && std::abs(endgame_score) > 250) {
		late_eg_score = utility::sgn(endgame_score) * (int)(4.7 * center_manhattan_distance[(endgame_score > 0) ? bkp : wkp] + 1.6 * (14 - manhattan_distance[wkp][bkp]));
		
	}

	//******************************************************************//
	// 																	//
	// combine endgame and midgame scores								//
	//																	//
	//******************************************************************//
	
	int score = (midgame_score * phase + (endgame_score + late_eg_score) * (76 - phase)) / 76 + eval_pawn_structure(b);

	// scores need to be from the perspective of the side to move
	// => so they are flipped if necessary
	return (b->side_to_move) ? -score : score;
}

int evaluator::eval_pawn_structure(bitboard* b)
{
	bit_move m;
	int hash_score = pawn_tt.probe_qsearch(b->pawn_hash_key, 0, -MATE, MATE, &m);
	if (hash_score != transposition_table::VAL_UNKNOWN) {
		return hash_score;
	}

	uint64_t w_pawns = b->bbs[PAWN][WHITE];
	uint64_t b_pawns = b->bbs[PAWN][BLACK];

	int score = 0;
	bool open_files[2][8] = { {false} };
	bool passed_pawns[2][8] = { {false} };
	
	
	//******************************************************************//
	// 																	//
	// double pawn penalty + half open files							//
	//																	//
	// computes a penalty for double pawns, and checks which files are	// 
	// half open at the same time										//
	// 																	//
	//******************************************************************//
	
	for (int i = 0; i < 8; i++) {
		uint64_t file = files[i];
		if ((w_pawns & file) != 0ULL) {
			score -= (int)(__popcnt64(w_pawns & file) - 1) * 25;
		}
		else {
			open_files[WHITE][i] = true;
		}
		if ((b_pawns & file) != 0ULL) {
			score += (int)(__popcnt64(w_pawns & file) - 1) * 25;
		}
		else {
			open_files[BLACK][i] = true;
		}
	}

	
	// initialize front/back array
	unsigned long index = 0;
	
	

	pawn_tt.set(b->pawn_hash_key, 0, 0, score, tt_entry::EXACT, bit_move());

	return score;
}

void evaluator::init_tables()
{
	for (int i = 0; i < 64; i++)
	{
		piece_square_tables[MIDGAME][WHITE_PAWN][i] = pawn_pst_mg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[MIDGAME][BLACK_PAWN][i] = -pawn_pst_mg[i];
		piece_square_tables[ENDGAME][WHITE_PAWN][i] = pawn_pst_eg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[ENDGAME][BLACK_PAWN][i] = -pawn_pst_eg[i];

		piece_square_tables[MIDGAME][WHITE_KNIGHT][i] = knight_pst_mg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[MIDGAME][BLACK_KNIGHT][i] = -knight_pst_mg[i];
		piece_square_tables[ENDGAME][WHITE_KNIGHT][i] = knight_pst_eg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[ENDGAME][BLACK_KNIGHT][i] = -knight_pst_eg[i];

		piece_square_tables[MIDGAME][WHITE_BISHOP][i] = bishop_pst_mg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[MIDGAME][BLACK_BISHOP][i] = -bishop_pst_mg[i];
		piece_square_tables[ENDGAME][WHITE_BISHOP][i] = bishop_pst_eg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[ENDGAME][BLACK_BISHOP][i] = -bishop_pst_eg[i];

		piece_square_tables[MIDGAME][WHITE_ROOK][i] = rook_pst_mg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[MIDGAME][BLACK_ROOK][i] = -rook_pst_mg[i];
		piece_square_tables[ENDGAME][WHITE_ROOK][i] = rook_pst_eg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[ENDGAME][BLACK_ROOK][i] = -rook_pst_eg[i];

		piece_square_tables[MIDGAME][WHITE_QUEEN][i] = queen_pst_mg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[MIDGAME][BLACK_QUEEN][i] = -queen_pst_mg[i];
		piece_square_tables[ENDGAME][WHITE_QUEEN][i] = queen_pst_eg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[ENDGAME][BLACK_QUEEN][i] = -queen_pst_eg[i];

		piece_square_tables[MIDGAME][WHITE_KING][i] = king_pst_mg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[MIDGAME][BLACK_KING][i] = -king_pst_mg[i];
		piece_square_tables[ENDGAME][WHITE_KING][i] = king_pst_eg[(7 - i / 8) * 8 + i % 8];
		piece_square_tables[ENDGAME][BLACK_KING][i] = -king_pst_eg[i];
	}

	// initialize manhattan distance table
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			manhattan_distance[i][j] = std::abs((i / 8) - (j / 8)) + std::abs((i % 8) - (j % 8));
		}
	}

	// initialize spans
	for (int i = 0; i < 64; i++) {
		int forward = i + 8;
		int backward = i - 8;
		uint64_t front_span = 0ULL;
		for (; forward < 64; forward += 8) {
			front_span |= 1ULL << forward;
		}
		uint64_t back_span = 0ULL;
		for (; backward >= 0; backward -= 8) {
			back_span |= 1ULL << backward;
		}
		front_spans[WHITE][i] = front_span;
		front_spans[BLACK][i] = back_span;
	}

	for (int i = 0; i < 64; i++) {
		uint64_t front_span = 0ULL;
		uint64_t back_span = 0ULL;
		if (i % 8 != 0) {
			int forward = i + 7;
			int backward = i - 9;
			for (; forward < 64; forward += 8) {
				front_span |= 1ULL << forward;
			}
			for (; backward >= 0; backward -= 8) {
				back_span |= 1ULL << backward;
			}
		}
		if (i % 8 != 7) {
			int forward = i + 9;
			int backward = i - 7;
			for (; forward < 64; forward += 8) {
				front_span |= 1ULL << forward;
			}
			for (; backward >= 0; backward -= 8) {
				back_span |= 1ULL << backward;
			}
		}
		attack_front_spans[WHITE][i] = front_span;
		attack_front_spans[BLACK][i] = back_span;
	}
}


const int evaluator::pawn_pst_mg[64] = {
	 0,   0,   0,   0,   0,   0,  0,   0,
	 98, 134,  61,  95,  68, 126, 34, -11,
	 -6,   7,  26,  31,  65,  56, 25, -20,
	-14,  13,   6,  21,  23,  12, 17, -23,
	-27,  -2,  -5,  12,  17,   6, 10, -25,
	-26,  -4,  -4, -10,   3,   3, 33, -12,
	-35,  -1, -20, -23, -15,  24, 38, -22,
	  0,   0,   0,   0,   0,   0,  0,   0,
};

const int evaluator::pawn_pst_eg[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	 94, 100,  85,  67,  56,  53,  82,  84,
	 32,  24,  13,   5,  -2,   4,  17,  17,
	 13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	  4,   7,  -6,   1,   0,  -5,  -1,  -8,
	 13,   8,   8,  10,  13,   0,   2,  -7,
	  0,   0,   0,   0,   0,   0,   0,   0,
};


const int evaluator::knight_pst_mg[64] = {
	-167, -89, -34, -49,  61, -97, -15, -107,
	 -73, -41,  72,  36,  23,  62,   7,  -17,
	 -47,  60,  37,  65,  84, 129,  73,   44,
	  -9,  17,  19,  53,  37,  69,  18,   22,
	 -13,   4,  16,  13,  28,  19,  21,   -8,
	 -23,  -9,  12,  10,  19,  17,  25,  -16,
	 -29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23
};

const int evaluator::knight_pst_eg[64] = {
	-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,
};

const int evaluator::bishop_pst_mg[64] = {
	-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	 -4,   5,  19,  50,  37,  37,   7,  -2,
	 -6,  13,  13,  26,  34,  12,  10,   4,
	  0,  15,  15,  15,  14,  27,  18,  10,
	  4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,
};

const int evaluator::bishop_pst_eg[64] = {
	-14, -21, -11,  -8, -7,  -9, -17, -24,
	 -8,  -4,   7, -12, -3, -13,  -4, -14,
	  2,  -8,   0,  -1, -2,   6,   0,   4,
	 -3,   9,  12,   9, 14,  10,   3,   2,
	 -6,   3,  13,  19,  7,  10,  -3,  -9,
	-12,  -3,   8,  10, 13,   3,  -7, -15,
	-14, -18,  -7,  -1,  4,  -9, -15, -27,
	-23,  -9, -23,  -5, -9, -16,  -5, -17,
};

const int evaluator::rook_pst_mg[64] = {
	 32,  42,  32,  51, 63,  9,  31,  43,
	 27,  32,  58,  62, 80, 67,  26,  44,
	 -5,  19,  26,  36, 17, 45,  61,  16,
	-24, -11,   7,  26, 24, 35,  -8, -20,
	-36, -26, -12,  -1,  9, -7,   6, -23,
	-45, -25, -16, -17,  3,  0,  -5, -33,
	-44, -16, -20,  -9, -1, 11,  -6, -71,
	-19, -13,   1,  17, 16,  7, -37, -26,
};

const int evaluator::rook_pst_eg[64] = {
	13, 10, 18, 15, 12,  12,   8,   5,
	11, 13, 13, 11, -3,   3,   8,   3,
	 7,  7,  7,  5,  4,  -3,  -5,  -3,
	 4,  3, 13,  1,  2,   1,  -1,   2,
	 3,  5,  8,  4, -5,  -6,  -8, -11,
	-4,  0, -5, -1, -7, -12,  -8, -16,
	-6, -6,  0,  2, -9,  -9, -11,  -3,
	-9,  2,  3, -1, -5, -13,   4, -20,
};

const int evaluator::queen_pst_mg[64] = {
	-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	 -9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	 -1, -18,  -9,  10, -15, -25, -31, -50,
};

const int evaluator::queen_pst_eg[64] = {
	 -9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	  3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,
};

const int evaluator::king_pst_mg[64] = {
	-65,  23,  16, -15, -56, -34,   2,  13,
	 29,  -1, -20,  -7,  -8,  -4, -38, -29,
	 -9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	  1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14
};
