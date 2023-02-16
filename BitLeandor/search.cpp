#include "search.h"

// defining static variables

bit_move search::PV[MAX_PV_SIZE][MAX_PV_SIZE] = {};
bit_move search::killers[2][MAX_PV_SIZE] = {};
int search::history[2][64][64] = { {{0}} };
int search::DEPTH = 0;
std::chrono::time_point<std::chrono::steady_clock> search::ENDTIME = {};
std::chrono::time_point<std::chrono::steady_clock> search::STARTTIME = {};
bool search::stop_now = false;

movelist search::moves[256] = {};
scorelist search::scores[256] = {};
transposition_table search::tt = transposition_table();

int search::NODES_SEARCHED = 0;
int search::QNODES_SEARCHED = 0;

const int search::mvv_lva[6][6] = {
	{105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600} };

int search::lmr[MAX_PV_SIZE][MAX_PV_SIZE] = { {0} };

int search::quiescence(bitboard* b, int alpha, int beta, int ply)
{
	NODES_SEARCHED++;
	QNODES_SEARCHED++;

	bit_move hash_move = bit_move();
	/*int hash_score = tt.probe_qsearch(b->zobrist_key, ply, alpha, beta, &hash_move);
	if (hash_score != tt.VAL_UNKNOWN)
	{
		return hash_score;
	}*/

	unsigned long king_pos;
	_BitScanForward64(&king_pos, b->bbs[KING][b->side_to_move]);
	
	bool is_check = b->is_square_attacked(king_pos, !b->side_to_move);

	int stand_pat;

	if (is_check) {
		stand_pat = -MATE + ply;
	}
	else {
		stand_pat = evaluator::eval(b);
	}

	if (stand_pat >= beta)
	{
		return beta;
	}
	// delta pruning
	// int big_delta = 950;
	// bool promotion_possible = false;
	// if (b->side_to_move) {
	// 	promotion_possible = b->pawns_before_back_rank<true>();
	// }
	// else {
	// 	promotion_possible = b->pawns_before_back_rank<false>();
	// }
	// if (promotion_possible) {
	// 	big_delta += 750;
	// }
	// if (stand_pat + big_delta < alpha) {
	// 	return alpha;
	// }

	if (stand_pat > alpha)
	{
		alpha = stand_pat;
	}

	int num_legal = 0;
	moves[ply].size = 0;
	scores[ply].size = 0;
	if (is_check)
	{
		movegen::generate_all_pseudo_legal_moves(b, &moves[ply]);
	}
	else
	{
		movegen::generate_all_captures(b, &moves[ply]);
	}

	uint8_t flag = tt_entry::UPPER_BOUND;
	bit_move best_move = bit_move();

	int size = moves[ply].size;
	score_moves(&moves[ply], &scores[ply], b->side_to_move);
	for (int i = 0; i < size; i++)
	{
		fetch_next_move(&moves[ply], &scores[ply], i);
		bit_move m = moves[ply].moves[i];
		if (b->is_legal<true>(&m))
		{
			num_legal++;
			int delta = alpha - stand_pat - 250;
			if (m.get_flags() >= 4 && evaluator::piece_values[0][b->types[m.get_target()]] < delta)
			{
				continue;
			}
			b->make_move(&m);

			int score = -quiescence(b, -beta, -alpha, ply + 1);

			b->unmake_move();

			if (score >= beta)
			{
				// tt.set(b->zobrist_key, 0, ply, score, tt_entry::LOWER_BOUND, m);
				return score;
			}

			if (score > alpha)
			{
				alpha = score;
				flag = tt_entry::EXACT;
				best_move = m;
			}
		}
	}
	if (is_check && num_legal == 0)
	{
		return -MATE + ply;
	}
	// tt.set(b->zobrist_key, 0, ply, alpha, flag, best_move);
	return alpha;
}

void search::init_lmr()
{
	for (int d = 0; d < 256; d++)
	{
		for (int m = 0; m < 256; m++)
		{
			lmr[d][m] = (int)(1.25 + log(d) * log(m) * 100 / 267);
			// std::cout << lmr[d][m] << ", ";
		}
		// std::cout << std::endl;
	}
}

int search::alpha_beta(bitboard* b, int depth, int alpha, int beta, int ply)
{
	if (stop_now)
	{
		return alpha;
	}

	if (NODES_SEARCHED % 2048 == 0)
	{
		communicate();
	}

	unsigned long king_pos;
	bool side_to_move = b->side_to_move;
	_BitScanForward64(&king_pos, b->bbs[KING][side_to_move]);
	bool is_check = b->is_square_attacked(king_pos, !side_to_move);

	//**********************************************************//
	//															//
	// check extension											//
	//															//
	//**********************************************************//

	depth += is_check;

	//**********************************************************//
	//															//
	// drop into quiescence search								//
	//															//
	//**********************************************************//

	if (depth == 0)
	{
		return quiescence(b, alpha, beta, ply);
	}

	// if we dont drop into qsearch, increment node counter
	NODES_SEARCHED++;

	// check for draw by repetition
	uint8_t rep_count = 1;
	for (size_t i = b->game_history.size(); i > 0; i--)
	{
		if (b->game_history[i - 1].last_move.move == 0 || b->game_history[i - 1].last_move.get_flags() != bit_move::quiet_move)
		{
			break;
		}
		if (b->game_history[i - 1].z_hash == b->zobrist_key)
		{
			rep_count++;
		}
		if (rep_count == 3)
		{
			return 0;
		}
	}

	// check for draw by 50 move rule
	if (b->fifty_move_rule_counter >= 100)
	{
		return 0;
	}

	bool pv = (beta - alpha) != 1;
	uint8_t num_legal = 0;
	// uint8_t num_quiet = 0;
	uint8_t flag = tt_entry::UPPER_BOUND;
	int best_score = -MATE + ply;

	uint8_t LMR = 0;

	bit_move best_move = bit_move();
	bit_move hash_move = bit_move();

	int hash_score = tt.probe(b->zobrist_key, depth, ply, alpha, beta, &hash_move);

	if (hash_score != tt_entry::VAL_UNKNOWN && ply != 0 && !pv)
	{
		return hash_score;
	}

	int static_eval = evaluator::eval(b);

	//**********************************************************//
	//															//
	// razoring													//
	//															//
	//**********************************************************//

	/*if (!pv && !is_check) {
		if (depth <= 3 && static_eval + 250 * depth < beta) {
			int score = quiescence(b, alpha, beta, ply);
			if (score < beta)
				return score;
		}
	}*/


	bool post_null_move = b->game_history.size() > 0 && b->game_history.back().last_move.move == 0;
	bool has_only_pawns = b->occupancy[side_to_move] == (b->bbs[PAWN][side_to_move] | b->bbs[KING][side_to_move]);

	//**********************************************************//
	//															//
	// mate dist pruning										//
	//															//
	//**********************************************************//
	{
		int mating_value = MATE - ply;
		if (mating_value < beta)
		{
			beta = mating_value;
			if (alpha >= mating_value)
			{
				return mating_value;
			}
		}

		mating_value = -MATE + ply;

		if (mating_value > alpha)
		{
			alpha = mating_value;
			if (beta <= mating_value)
			{
				return mating_value;
			}
		}
	}

	//**********************************************************//
	//															//
	// null move pruning										//
	//															//
	//**********************************************************//

	if (depth > 2 && !is_check && !has_only_pawns && !pv && !post_null_move && static_eval >= beta)
	{
		b->make_null_move();
		int R = 2;
		if (depth > 6)
		{
			R++;
		}
		if (depth > 10)
		{
			R++;
		}

		int null_score = -quiescence(b, -beta, -beta + 1, ply + 1);
		b->unmake_null_move();
		if (null_score >= beta)
		{
			return beta;
		}
	}

	//******************************************************//
	//														//
	// try the hash move first								//
	//														//
	//******************************************************//

	uint8_t step = 0;
	uint8_t num_quiets = 0;
	bool generate_quiets = false;

non_generated_moves:
	if (hash_score != transposition_table::VAL_UNKNOWN || step > 0)
	{
		if (b->is_legal<false>(&hash_move))
		{
			num_legal++;
			num_quiets += hash_move.get_flags() < 4;
			b->make_move(&hash_move);
			int score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
			b->unmake_move();
			if (score >= beta)
			{
				tt.set(b->zobrist_key, depth, ply, beta, tt_entry::LOWER_BOUND, hash_move);
				return score;
			}
			if (score > alpha)
			{
				alpha = score;
				// update PV
			}
			if (score > best_score)
			{
				best_score = score;
				flag = tt_entry::EXACT;
				best_move = hash_move;
				update_PV(&hash_move, ply);
			}
		}
	}

generate:
	if (generate_quiets && step < 2)
	{
		hash_move = killers[step][ply];
		step++;
		goto non_generated_moves;
	}

	moves[ply].size = 0;
	scores[ply].size = 0;

	if (!generate_quiets)
	{
		movegen::generate_all_captures(b, &moves[ply]);
	}
	else
	{
		movegen::generate_all_quiet_moves(b, &moves[ply]);
	}
	score_moves(&moves[ply], &scores[ply], side_to_move);

	for (int i = 0; i < moves[ply].size; i++)
	{
		fetch_next_move(&moves[ply], &scores[ply], i);
		bit_move m = moves[ply].moves[i];

		if (b->is_legal<true>(&m))
		{
			num_legal++;
			uint8_t move_flags = m.get_flags();
			if (move_flags < 4 && !is_check && !pv)
			{
				// futility pruning
				if (flag == tt_entry::EXACT && depth == 1) {
					if (static_eval + 250 < alpha) {
						continue;
					}
				}

				// extended futility pruning
				if (flag == tt_entry::EXACT && depth == 2) {
					if (static_eval + 450 < alpha) {
						// prune branch
						continue;
					}
				}

				// late move leaf pruning
				// if (flag == tt_entry::EXACT && depth == 1)
				// {
				// 	if (num_quiets > 7)
				// 	{
				// 		continue;
				// 	}
				// }
				num_quiets++;
			}
			LMR = 0 + (!(m.get_flags() >= 4) && (num_legal > 1)) * (lmr[depth - 1][num_legal]);

			b->make_move(&m);
			int score;
			if (flag == tt_entry::EXACT)
			{
				score = -alpha_beta(b, depth - 1 - LMR, -alpha - 1, -alpha, ply + 1);
				if (score > alpha && score < beta)
				{
					score = -alpha_beta(b, depth - 1, -alpha - 1, -alpha, ply + 1);
				}
				if (score > alpha && score < beta)
				{ // research at full depth
					score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
				}
			}
			else
			{
				score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
			}

			b->unmake_move();
			if (score >= beta)
			{
				if (m.get_flags() < 4)
				{
					history[b->side_to_move][m.get_origin()][m.get_target()] += ply * ply;

					killers[1][ply] = killers[0][ply];
					killers[0][ply] = m;
				}
				tt.set(b->zobrist_key, depth, ply, score, tt_entry::LOWER_BOUND, m);
				return score;
			}
			if (score > best_score)
			{
				best_score = score;
				best_move = m;
				flag = tt_entry::EXACT;
			}
			if (score > alpha)
			{
				alpha = score;
				update_PV(&m, ply);
			}
		}
	}
	if (!generate_quiets)
	{
		generate_quiets = true;
		goto generate;
	}

	if (num_legal == 0 && is_check)
	{
		best_score = -MATE + ply;
		flag = tt_entry::EXACT;
	}
	else if (num_legal == 0)
	{
		best_score = 0;
	}
	tt.set(b->zobrist_key, depth, ply, best_score, flag, best_move);
	return best_score;
}

int search::search_iterative_deepening(bitboard* b, int depth, bool quiet)
{
	stop_now = false;
	int prev_score = 0;
	bit_move best_move = bit_move();
	int i;
	int score = 0;
	int TOTAL_NODES = 0;
	//**********************************************************//
	// 															//
	// iterate from depth 1 to n								//
	// 															//
	//**********************************************************//

	STARTTIME = std::chrono::high_resolution_clock::now();

	for (i = 1; i <= depth; i++)
	{
		NODES_SEARCHED = 0;
		QNODES_SEARCHED = 0;
		score = 0;
		int num_fails = 0;

		// if depth == 1, search without aspiration windows

		if (i == 1)
		{
			score = alpha_beta(b, i, -MATE, MATE, 0);
			prev_score = score;
		}
		else
		{
			// set aspiration window
			int prev_alpha = prev_score - 16;
			int prev_beta = prev_score + 16;

			// search with aspiration window
			score = alpha_beta(b, i, prev_alpha, prev_beta, 0);

			num_fails = 0;

			// as long as the score is outside the window, reset PV + bestmove and search again
			while ((score <= prev_alpha || score >= prev_beta) && !stop_now)
			{
				for (int x = 0; x < MAX_PV_SIZE; x++)
				{
					for (int y = 0; y < MAX_PV_SIZE; y++)
					{
						PV[x][y].move = 0;
					}
				}
				// if the score is too low, decrease alpha
				prev_alpha = (score <= prev_alpha) ? prev_alpha - (16 << num_fails) : prev_alpha;
				// if the score is too high, increase beta
				prev_beta = (score >= prev_beta) ? prev_beta + (16 << num_fails) : prev_beta;
				// research
				score = alpha_beta(b, i, prev_alpha, prev_beta, 0);
				num_fails++;
			}
			if (!stop_now)
			{
				prev_score = score;
			}
		}

		if (stop_now)
		{
			// if we stopped the search, abort, since incomplete search is invalid
			if (!quiet)
				std::cout << "info nodes " << NODES_SEARCHED << std::endl;
			break;
		}
		else
		{
			// if we didn't stop the search, print the info
			if (!quiet) {
				std::cout << "info depth " << i << " seldepth " << i << " multipv 1";

				if (std::abs(score) < MATE - MAX_PV_SIZE - 1 && !stop_now)
				{
					std::cout << " score cp " << ((stop_now) ? prev_score : score) << " ";
				}
				else
				{
					std::cout << " score mate " << utility::sgn(score) * (MATE - std::abs(score) + 1) / 2 << " ";
				}

				best_move = PV[0][0];
				std::cout << "pv ";
				int j;
				int dist_to_mate = std::abs(MATE - score);
				bool root_side = b->side_to_move;
				for (j = 0; j < std::min(i, dist_to_mate); j++)
				{
					bit_move m = PV[0][j];
					if (m.move == 0)
					{
						break;
					}
					if (!b->is_legal<false>(&m) && j < i - 1)
					{
						break;
					}

					tt.set(b->zobrist_key, i - j, j, (b->side_to_move == root_side) ? score : -score, tt_entry::EXACT, m);
					b->make_move(&m);
					std::cout << bit_move::to_string(search::PV[0][j]) << " ";
				}
				for (; j > 0; j--)
				{
					b->unmake_move();
				}

				std::cout << "nodes " << NODES_SEARCHED << " qnodes " << QNODES_SEARCHED << " ";

				int duration_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - STARTTIME).count();
				// compute nps
				int nps = (duration_ms > 0) ? (int)((double)NODES_SEARCHED / ((double)duration_ms / 1000.0)) : 0;
				std::cout << "nps " << nps << " num_fails " << num_fails << std::endl;
			}
			for (int x = 0; x < MAX_PV_SIZE; x++)
			{
				for (int y = 0; y < MAX_PV_SIZE; y++)
				{
					PV[x][y].move = 0;
				}
			}

			TOTAL_NODES += NODES_SEARCHED;
		}
	}
	if (!quiet)
		std::cout << "bestmove " << bit_move::to_string(best_move) << std::endl;
	return TOTAL_NODES;
}

void search::update_PV(bit_move* m, int ply)
{
	PV[ply][0] = *m;
	for (int i = 0; i < MAX_PV_SIZE - 1; i++)
	{
		PV[ply][i + 1] = PV[ply + 1][i];
	}
}

void search::score_moves(movelist* m_l, scorelist* s_l, bool side_to_move)
{
	s_l->size = m_l->size;
	for (int i = 0; i < m_l->size; i++)
	{
		bit_move m = m_l->moves[i];
		if (m.get_flags() >= 4)
		{
			s_l->scores[i] = mvv_lva[m.get_piece_type()][m.get_captured_type()];
		}
		else
		{
			s_l->scores[i] = history[side_to_move][m.get_origin()][m.get_target()];
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
void search::fetch_next_move(movelist* m_l, scorelist* s_l, int index)
{
	// defining temp variables to store the move and score
	int max_score = s_l->scores[index];
	int original_score_index = index;
	for (int i = index; i < m_l->size; i++)
	{
		if (s_l->scores[i] > max_score)
		{
			max_score = s_l->scores[i];
			original_score_index = i;
		}
	}
	if (original_score_index != index)
	{
		bit_move max_move = m_l->moves[original_score_index];
		m_l->moves[original_score_index] = m_l->moves[index];
		m_l->moves[index] = max_move;
		s_l->scores[original_score_index] = s_l->scores[index];
		s_l->scores[index] = max_score;
	}
}

void search::communicate()
{
	if (ENDTIME.time_since_epoch().count() != 0LL && std::chrono::high_resolution_clock().now().time_since_epoch().count() > ENDTIME.time_since_epoch().count())
	{
		stop_now = true;
		return;
	}
	if (input_waiting())
	{
		std::string line = "";
		std::getline(std::cin, line);
		if (line != "")
		{
			std::vector<std::string>* split = new std::vector<std::string>;
			utility::split_string(split, line);
			if (split->at(0) == "stop")
			{
				stop_now = true;
				return;
			}
		}
	}
}
