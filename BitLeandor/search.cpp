#include "search.h"

bit_move search::PV[2080] = {};
int search::NODES_SEARCHED = 0;

inline int search::get_pv_index(int depth_left)
{
	return (depth_left * depth_left) >> 1; // depth * depth / 2
}

int search::quiescence(bitboard* b, int alpha , int beta, int ply)
{
	// if is check
	NODES_SEARCHED++;
	unsigned long king_pos;
	_BitScanForward64(&king_pos, b->bbs[bitboard::KING][b->side_to_move]);
	movelist l;
	bool is_check = b->is_square_attacked(king_pos, !b->side_to_move);
	if (is_check) {
		movegen::generate_all_pseudo_legal_moves(b, &l);
	}
	else {
		movegen::generate_all_captures(b, &l);
	}
	int stand_pat = evaluator::eval_material(b);
	if (stand_pat >= beta) {
		return beta;
	}
	if (stand_pat > alpha) {
		alpha = stand_pat;
	}
	int num_legal = 0;
	for (int i = 0; i < l.size; i++) {
		bit_move m = l.moves[i];
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
		return evaluator::eval_material(b);
	}
	else return alpha;
}

int search::alpha_beta(bitboard *b, int depth, int alpha, int beta, int ply)
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
	movelist l;
	movegen::generate_all_pseudo_legal_moves(b, &l);

	int num_legal = 0;
	int best_score = -MATE;
	// fail soft alpha-beta search
	for (int i = 0; i < l.size; i++) {
		bit_move m = l.moves[i];
		if (b->is_legal<true>(&m)) {
			num_legal++;
			b->make_move(&m);
			int score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
			b->unmake_move();
			if (score >= beta) {
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
	return best_score;
}

void search::update_PV(bit_move *m, int depth)
{
	int curr_index = get_pv_index(depth);
	PV[curr_index] = *m;
	int index = get_pv_index(depth - 1);
	for (int i = 0; i < depth - 1; i++) {
		PV[curr_index + i + 1] = PV[index + i];
	}
}
