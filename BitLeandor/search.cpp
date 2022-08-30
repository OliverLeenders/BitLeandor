#include "search.h"

bit_move search::PV[MAX_PV_SIZE][MAX_PV_SIZE] = {};
bit_move search::killers[2][MAX_PV_SIZE] = {};
int search::history[2][64][64] = { 0 };
int search::DEPTH = 0;
std::chrono::time_point<std::chrono::steady_clock> search::ENDTIME = {};
bool search::stop_now = false;

movelist search::moves[256] = {};
scorelist search::scores[256] = {};
transposition_table search::tt = transposition_table();

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
	_BitScanForward64(&king_pos, b->bbs[KING][b->side_to_move]);
	bool is_check = b->is_square_attacked(king_pos, !b->side_to_move);


	

	bit_move hash_move = bit_move();
	int hash_score = tt.probe_qsearch(b->zobrist_key, ply, alpha, beta, &hash_move);
	if (hash_score != tt_entry::VAL_UNKNOWN) {
		return hash_score;
	}
	
	int stand_pat = (is_check) ? -MATE - 1 : evaluator::eval(b);;
	if (stand_pat >= beta) {
		return beta;
	}
	if (stand_pat > alpha) {
		alpha = stand_pat;
	}
	
	bit_move best_move = bit_move();

	int num_legal = 0;
	uint8_t flag = tt_entry::UPPER_BOUND;

	if (hash_score != transposition_table::VAL_UNKNOWN) {

		if (b->is_legal<false>(&hash_move)) {
			num_legal++;
			int delta = 950 + (hash_move.get_flags() >= bit_move::knight_promotion) * 850;
			// delta pruning
			if (stand_pat < alpha - delta) {
				return alpha;
			}
			b->make_move(&hash_move);
			int score = -quiescence(b, -beta, -alpha, ply + 1);
			b->unmake_move();
			if (score >= beta) {
				tt.set(b->zobrist_key, 0, beta, tt_entry::LOWER_BOUND, hash_move);
				return score;
			}
			if (score > alpha) {
				alpha = score;
				// update PV
			}
			if (score > stand_pat) {
				stand_pat = score;
				flag = tt_entry::EXACT;
				best_move = hash_move;
			}
		}
	}
	if (ply == MAX_PV_SIZE) {
		b->print_board();
	}
	moves[ply].size = 0;
	scores[ply].size = 0;
	if (is_check) {
		movegen::generate_all_pseudo_legal_moves(b, &moves[ply]);
	}
	else {
		movegen::generate_all_captures(b, &moves[ply]);
	}

	int size = moves[ply].size;
	score_moves(&moves[ply], &scores[ply], ply, b->side_to_move);
	for (int i = 0; i < size; i++) {
		fetch_next_move(&moves[ply], &scores[ply], i);
		bit_move m = moves[ply].moves[i];
		if (b->is_legal<true>(&m)) {
			num_legal++;
			int delta = 950 + (m.get_flags() >= bit_move::knight_promotion) * 850;
			// delta pruning
			if (stand_pat < alpha - delta) {
				return alpha;
			}
			b->make_move(&m);
			//std::cout << bit_move::to_string(m) << " " <<alpha << " " << beta << std::endl;
			int score = -quiescence(b, -beta, -alpha, ply + 1);
			b->unmake_move();
			if (score >= beta) {
				tt.set(b->zobrist_key, 0, score, tt_entry::LOWER_BOUND, m);
				return score;
			}
			if (score > alpha) {
				alpha = score;
			}
			if (score > stand_pat) {
				stand_pat = score;
				best_move = m;
				flag = tt_entry::EXACT;
			}
		}
	}
	if (is_check && num_legal == 0) {
		return -MATE;
	}
	if (num_legal == 0) {
		int score = evaluator::eval(b);
		tt.set(b->zobrist_key, 0, score, flag, best_move);
		return score;
	}
	else {
		return alpha;
	};
}

int search::alpha_beta(bitboard* b, int depth, int alpha, int beta, int ply)
{
	if (NODES_SEARCHED % 2048 == 0) {
		communicate();
	}

	if (stop_now) {
		return alpha;
	}

	NODES_SEARCHED++;
	unsigned long king_pos;
	_BitScanForward64(&king_pos, b->bbs[KING][b->side_to_move]);
	bool is_check = b->is_square_attacked(king_pos, !b->side_to_move);
	// if is check and depth == 0 increase depth by 1
	depth += is_check && depth == 0;

	if (depth == 0) {
		return quiescence(b, alpha, beta, ply);
	}

	bool pv = (beta - alpha) != 1;
	bit_move hash_move;
	
	int hash_score = tt.probe(b->zobrist_key, depth, ply, alpha, beta, &hash_move);


	if (ply != 0 && hash_score != transposition_table::VAL_UNKNOWN && !pv) {
		return hash_score;
	}

	int static_eval = evaluator::eval(b);

	// razoring (at this point taken from Koivsito's search)
	// not used because it cripples the PV for some reason
	if (depth <= 3 && !pv && !is_check) {

		if (static_eval + 190 * depth < beta) {
			int razor_score = quiescence(b, alpha + 1, beta, ply);
			if (razor_score - 190 < beta) {
				return razor_score;
			}
			else if (depth == 1) {
				return beta;
			}
		}
	}

	// static null move pruning
	if (depth <= 5 && !is_check && !pv) {
		if (static_eval > beta + depth * 190) {
			return static_eval;
		}
	}


	// null move pruning
	if (depth > 2 && !is_check && (b->game_history.size() == 0 || b->game_history.back().last_move.move != 0) && (b->bbs[KNIGHT][b->side_to_move] != 0ULL && b->bbs[BISHOP][b->side_to_move] != 0ULL)) {
		int8_t prev_ep = b->ep_target_square;
		b->ep_target_square = -1;
		if (prev_ep != -1) {
			b->zobrist_key ^= prev_ep % 8;
		}
		b->make_null_move();
		int null_score = -alpha_beta(b, depth - 3, -beta, -beta + 1, ply + 1);
		b->unmake_null_move();
		b->ep_target_square = prev_ep;
		if (prev_ep != -1) {
			b->zobrist_key ^= prev_ep % 8;
		}
		if (null_score >= beta) {
			return beta;
		}
	}
	int best_score = -MATE - 1;
	bit_move best_move = bit_move();
	uint8_t flag = tt_entry::UPPER_BOUND;




	int num_legal = 0;
	int num_quiets = 0;

	// try hash move first
	if (hash_score != transposition_table::VAL_UNKNOWN) {

		if (b->is_legal<false>(&hash_move)) {
			num_legal++;
			if (hash_move.get_flags() < 4) {
				num_quiets++;
			}

			b->make_move(&hash_move);
			int score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
			b->unmake_move();
			if (score >= beta) {
				if (hash_move.get_flags() < 4) {
					history[b->side_to_move][hash_move.get_origin()][hash_move.get_target()] += ply * ply;

					killers[1][ply] = killers[0][ply];
					killers[0][ply] = hash_move;
				}
				tt.set(b->zobrist_key, depth, beta, tt_entry::LOWER_BOUND, hash_move);

				return score;
			}
			if (score > alpha) {
				alpha = score;
				// update PV
				update_PV(&hash_move, ply);
			}
			if (score > best_score) {
				best_score = score;
				best_score = score;
				best_move = hash_move;
				flag = tt_entry::EXACT;
			}
		}
	}

	moves[ply].size = 0;
	scores[ply].size = 0;
	movegen::generate_all_pseudo_legal_moves(b, &moves[ply]);


	int size = moves[ply].size;
	int lmr = 0;
	// fail soft alpha-beta search
	score_moves(&moves[ply], &scores[ply], ply, b->side_to_move);
	for (int i = 0; i < size; i++) {
		fetch_next_move(&moves[ply], &scores[ply], i);
		bit_move m = moves[ply].moves[i];
		if (b->is_legal<true>(&m)) {
			num_legal++;
			// quiet move pruning
			if (m.get_flags() < 4) {
				// late move pruning
				if (depth <= 7
					&& !is_check
					&& num_quiets >= 7
					&& flag == tt_entry::EXACT) {
					continue;
				}

				// prune moves which are unlikely to improve alpha (idea from Koivisto) => futility pruning?
				if (static_eval + 190 * depth < alpha
					&& !is_check
					&& flag == tt_entry::EXACT && depth <= 3) {
					continue;
				}


				// history based pruning (idea from koivisto)
				if (history[b->side_to_move][m.get_origin()][m.get_target()] < std::min(150 - depth * depth, 0)
					&& flag == tt_entry::EXACT) {
					continue;
				}

				num_quiets++;
			}
			if (m.get_flags() < bit_move::capture && !is_check) {
				lmr = std::min(depth - 1, int(std::sqrt(double(depth - 1)) + std::sqrt(double(num_legal - 1))));
				if (pv) {
					lmr *= 2 / 3;
				}
				if (b->game_history.back().last_move.move == 0) {
					lmr++;
				}
			}
			int score;
			b->make_move(&m);
			if (num_legal > 1) {
				score = -alpha_beta(b, depth - 1 - lmr, -alpha - 1, -alpha, ply + 1);
				if (score > alpha && score < beta) {
					score = -alpha_beta(b, depth - 1 - lmr, -beta, -alpha, ply + 1);
				}
				if (score > alpha) {
					score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
				}
			}
			else {
				score = -alpha_beta(b, depth - 1 - lmr, -beta, -alpha, ply + 1);
				if (score > alpha) {
					score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
				}
			}
			b->unmake_move();

			if (score >= beta) {
				if (hash_move.get_flags() < 4) {
					history[b->side_to_move][m.get_origin()][m.get_target()] += ply * ply;

					killers[1][ply] = killers[0][ply];
					killers[0][ply] = m;
				}
				tt.set(b->zobrist_key, depth, beta, tt_entry::LOWER_BOUND, m);

				return score;
			}
			if (score > alpha) {
				alpha = score;
				best_score = score;
				best_move = m;
				flag = tt_entry::EXACT;
				// update PV
				update_PV(&m, ply);
			}
			if (score > best_score) {
				best_score = score;
				best_move = m;
			}
		}
	}
	if (is_check && num_legal == 0) {
		return -MATE;
	}
	else if (num_legal == 0) {
		return 0;
	}
	tt.set(b->zobrist_key, depth, best_score, flag, best_move);
	return best_score;
}

int search::search_iterative_deepening(bitboard* b, int depth)
{
	stop_now = false;
	int prev_score = 0;
	bit_move best_move = bit_move();
	int i;
	for (i = 1; i <= depth; i++) {
		NODES_SEARCHED = 0;
		int score;
		if (i > 1) {
			score = alpha_beta(b, i, prev_score - 75, prev_score + 75, 0);
			if (std::abs(prev_score - score) >= 75 && !stop_now) {
				// std::cout << "aspiration fail" << std::endl;
				for (int x = 0; x < MAX_PV_SIZE; x++) {
					for (int y = 0; y < MAX_PV_SIZE; y++) {
						PV[x][y].move = 0;
					}
				}
				score = alpha_beta(b, i, -MATE, MATE, 0);
			}
		}
		else {
			score = alpha_beta(b, i, -MATE, MATE, 0);
		}
		prev_score = score;
		std::cout << "info depth " << i << " score cp " <<  ((!stop_now) ? score : prev_score) << " ";

		if (!stop_now) {
			best_move = PV[0][0];
			std::cout << "pv ";
			int j;
			for (j = 0; j < MAX_PV_SIZE; j++) {
				if (PV[0][j].move == 0) {
					break;
				}
				std::cout << bit_move::to_string(search::PV[0][j]) << " ";
				tt.set(b->zobrist_key, i - j, score, tt_entry::EXACT, search::PV[0][j]);
				b->make_move(&search::PV[0][j]);
			}
			for (; j > 0; j--) {
				b->unmake_move();
			}
		}
		std::cout << "nodes " << NODES_SEARCHED << std::endl;
		for (int x = 0; x < MAX_PV_SIZE; x++) {
			for (int y = 0; y < MAX_PV_SIZE; y++) {
				PV[x][y].move = 0;
			}
		}
		if (stop_now) {
			break;
		}
	}
	std::cout << "bestmove " << bit_move::to_string(best_move) << std::endl;
}

void search::update_PV(bit_move* m, int ply)
{
	PV[ply][0] = *m;
	for (int i = 0; i < MAX_PV_SIZE - 1; i++) {
		if (PV[ply + 1][i].move == 0) {
			break;
		}
		PV[ply][i + 1] = PV[ply + 1][i];
	}
}

void search::score_moves(movelist* m_l, scorelist* s_l, int ply, bool side_to_move)
{
	s_l->size = m_l->size;
	for (int i = 0; i < m_l->size; i++) {
		bit_move m = m_l->moves[i];
		if (m.get_flags() >= 4) {
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
/// Swaps the move with the highest score in the move list to the back of the list. Also swaps the scores.
/// Basically a selection sort.
/// </summary>
/// <param name="m_l">Movelist</param>
/// <param name="s_l">Scorelist</param>
/// <param name="index">Index pointing to next element after sorted area</param>
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

void search::communicate()
{
	if (input_waiting()) {
		std::string line = "";
		std::getline(std::cin, line);
		std::vector<std::string>* split = new std::vector<std::string>;
		Utility::split_string(split, line);
		if (split->at(0) == "stop") {
			stop_now = true;
		}
	}
	else if (ENDTIME.time_since_epoch().count() != 0LL 
		&& std::chrono::high_resolution_clock().now().time_since_epoch().count() > ENDTIME.time_since_epoch().count()) {
		stop_now = true;
	}
}
