#pragma once
#include "bitboard.h"
#include "evaluator.h"
#include "movelist.h"
#include "movegen.h"

class search
{
public:
	static bit_move PV[2080];
	static const int MATE = 100000;
	static const int MAX_PV_SIZE = 64;
	static int NODES_SEARCHED;
	static inline int get_pv_index(int depth_left);
	static int alpha_beta(bitboard* b, int depth, int alpha, int beta, int ply);
	static int quiescence(bitboard* b, int alpha, int beta, int ply);
	static void update_PV(bit_move* m, int depth);
private:

};
