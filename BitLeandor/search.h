#pragma once
#include "bitboard.h"
#include "evaluator.h"
#include "movelist.h"
#include "scorelist.h"
#include "movegen.h"

class search
{
public:
	static const int MATE = 100000;
	static const int MAX_PV_SIZE = 64;
	static const int CAPTURE_SCORE = 100000;
	static const int KILLER_1_SCORE = 10000;
	static const int KILLER_2_SCORE = 9999;

	static movelist moves[256];
	static scorelist scores[256];
	static int history[2][64][64];
	static bit_move killers[2][MAX_PV_SIZE];

	
	static bit_move PV[2080];

	static int NODES_SEARCHED;
	static inline int get_pv_index(int depth_left)  {
		return (depth_left * depth_left) >> 1; // depth_left * depth_left / 2
	}
	static int alpha_beta(bitboard* b, int depth, int alpha, int beta, int ply);
	static int quiescence(bitboard* b, int alpha, int beta, int ply);
	static void update_PV(bit_move* m, int depth);
	static const int mvv_lva[6][6];
	static void score_moves(movelist* m_l, scorelist* s_l, int ply, bool side_to_move);
	static void fetch_next_move(movelist* m_l, scorelist* s_l, int index);
private:

};
