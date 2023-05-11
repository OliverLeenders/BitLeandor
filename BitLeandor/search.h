#pragma once
#include <chrono>
#include <io.h>


#include "bitboard.h"
#include "evaluator.h"
#include "movelist.h"
#include "scorelist.h"
#include "movegen.h"
#include "transposition_table.h"
#include "uci_io.h"

class search
{
public:
	static const int MAX_PV_SIZE = 256;
	static const int MAX_DEPTH = 265;
	static const int MAX_PLY = 256;
	static const int CAPTURE_SCORE = 100000;
	static const int KILLER_1_SCORE = 10000;
	static const int KILLER_2_SCORE = 9999;
	static int lmr[MAX_PV_SIZE][MAX_PV_SIZE];

	static void init_lmr();

	static int DEPTH;
	static int SELDEPTH;
	static std::chrono::time_point<std::chrono::high_resolution_clock> ENDTIME;
	static std::chrono::time_point<std::chrono::high_resolution_clock> STARTTIME;
	static movelist moves[256];
	
	static int history[2][64][64];
	static void clear_history();

	static bit_move killers[2][MAX_PV_SIZE];
	static void clear_killers();

	static transposition_table tt;
	
	static bit_move PV[MAX_PLY][MAX_PV_SIZE];
	static int PV_SIZE[MAX_DEPTH];
	static int prev_pv_size;
	static void clear_PV();
	
	static void gather_and_print_pv(bitboard* b, int score, int curr_depth);

	static bool stop_now;

	static int NODES_SEARCHED;
	static int QNODES_SEARCHED;
	static inline int get_pv_index(int ply)  {
		return ((MAX_PV_SIZE * 2) + 1 - ply) / 2; // depth_left * depth_left / 2
	}
	static int alpha_beta(bitboard* b, int depth, int alpha, int beta, int ply);
	static int search_iterative_deepening(bitboard* b, int depth, bool quiet);
	static int quiescence(bitboard* b, int alpha, int beta, int ply);
	static void update_PV(bit_move* m, int depth);
	static const int mvv_lva[6][6];
	static void score_moves(movelist* m_l, bool side_to_move);
	static void fetch_next_move(movelist* m_l, int index);
	static void communicate();
private:

};
