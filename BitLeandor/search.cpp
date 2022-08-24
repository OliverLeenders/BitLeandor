#include "search.h"

bit_move search::PV[2080] = {};
bit_move search::killers[2][MAX_PV_SIZE] = {};
int search::history[2][64][64] = {0};

movelist search::moves[256] = {};
scorelist search::scores[256] = {};

int search::NODES_SEARCHED = 0;

const int search::mvv_lva[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600}
};



int search::quiescence(bitboard* b, int alpha, int beta, int ply)
{
	// if is check
	NODES_SEARCHED++;
	unsigned long king_pos;
	_BitScanForward64(&king_pos, b->bbs[bitboard::KING][b->side_to_move]);
	bool is_check = b->is_square_attacked(king_pos, !b->side_to_move);
	moves[ply].size = 0;
	scores[ply].size = 0;
	if (is_check) {
		movegen::generate_all_pseudo_legal_moves(b, &moves[ply]);
	}
	else {
		movegen::generate_all_captures(b, &moves[ply]);
	}
	int stand_pat = evaluator::eval(b);
	if (stand_pat >= beta) {
		return beta;
	}
	if (stand_pat > alpha) {
		alpha = stand_pat;
	}
	int num_legal = 0;
	int size = moves[ply].size;
	score_moves(&moves[ply], &scores[ply], ply, b->side_to_move);
	for (int i = 0; i < size; i++) {
		fetch_next_move(&moves[ply], &scores[ply], i);
		bit_move m = moves[ply].moves[i];
		if (b->is_legal<true>(&m)) {
			num_legal++;
			b->make_move(&m);
			//std::cout << bit_move::to_string(m) << " " <<alpha << " " << beta << std::endl;
			int score = -quiescence(b, -beta, -alpha, ply + 1);
			b->unmake_move();
			if (score >= beta) {
				return score;
			}
			if (score > alpha) {
				alpha = score;
			}
		}
	}
	if (is_check && num_legal == 0) {
		return MATE;
	}
	if (num_legal == 0) {
		return evaluator::eval(b);
	}
	else return alpha;
}

int search::alpha_beta(bitboard* b, int depth, int alpha, int beta, int ply)
{
	NODES_SEARCHED++;
	unsigned long king_pos;
	_BitScanForward64(&king_pos, b->bbs[bitboard::KING][b->side_to_move]);
	bool is_check = b->is_square_attacked(king_pos, !b->side_to_move);
	// if is check increase depth by 1
	depth += is_check;
	if (depth == 0) {
		return quiescence(b, alpha, beta, ply);
	}
	moves[ply].size = 0;
	scores[ply].size = 0;
	movegen::generate_all_pseudo_legal_moves(b, &moves[ply]);

	int num_legal = 0;
	int best_score = -MATE;
	int size = moves[ply].size;
	// fail soft alpha-beta search
	score_moves(&moves[ply], &scores[ply], ply, b->side_to_move);
	for (int i = 0; i < size; i++) {
		fetch_next_move(&moves[ply], &scores[ply], i);
		bit_move m = moves[ply].moves[i];
		if (b->is_legal<true>(&m)) {
			
			num_legal++;
			
			b->make_move(&m);
			int score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
			b->unmake_move();
			
			if (score >= beta) {
				history[b->side_to_move][m.get_origin()][m.get_target()] += ply * ply;
				
				killers[1][ply] = killers[0][ply];
				killers[0][ply] = m;
				
				return score;
			}
			if (score > alpha) {
				alpha = score;
				best_score = score;
				// update PV
				update_PV(&m, depth);
			}
		}
	}
	if (is_check && num_legal == 0) {
		return MATE;
	}
	else if (num_legal == 0) {
		return 0;
	}
	
	return best_score;
}

void search::update_PV(bit_move* m, int depth)
{
	int curr_index = get_pv_index(depth);
	PV[curr_index] = *m;
	int index = get_pv_index(depth - 1);
	for (int i = 0; i < depth - 1; i++) {
		PV[curr_index + i + 1] = PV[index + i];
	}
}

void search::score_moves(movelist* m_l, scorelist* s_l, int ply, bool side_to_move)
{
	s_l->size = m_l->size;
	for (int i = 0; i < m_l->size; i++) {
		bit_move m = m_l->moves[i];
		if (m.get_flags() >= 4)	{
			s_l->scores[i] = mvv_lva[m.get_piece_type()][m.get_captured_type()];
		}
		else {
			if (m.move == killers[0][ply].move) {
				s_l->scores[i] = KILLER_1_SCORE;
			}
			else if (m.move == killers[1][ply].move) {
				s_l->scores[i] = KILLER_2_SCORE;
			}
			else {
				s_l->scores[i] = history[side_to_move][m.get_origin()][m.get_target()];
			}
		}
	}
}

/// <summary>
/// Swaps the move with the highest score in the move list to the back of the list. Also swaps the scores
/// </summary>
/// <param name="m_l"></param>
/// <param name="s_l"></param>
/// <param name="index"></param>
void search::fetch_next_move(movelist* m_l, scorelist* s_l, int index) {
	// defining temp variables to store the move and score
	int max_score = s_l->scores[index];
	int original_score_index = index;
	for (int i = index; i < m_l->size; i++) {
		if (s_l->scores[i] > max_score) {
			max_score = s_l->scores[i];
			original_score_index = i;
		}
	}
	if (original_score_index != index) {
		bit_move max_move = m_l->moves[original_score_index];
		m_l->moves[original_score_index] = m_l->moves[index];
		m_l->moves[index] = max_move;
		s_l->scores[original_score_index] = s_l->scores[index];
		s_l->scores[index] = max_score;
	}
}