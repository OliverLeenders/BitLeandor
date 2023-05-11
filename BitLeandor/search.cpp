/**
 * @file search.cpp
 * @author Oliver Leenders (oliver.leenders@gmx.net)
 * @brief Contains all data declarations and function declarations needed for search.
 * @version 0.1
 * @date 2023-05-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "search.h"

//====================================================//
//
// defining static variables
//
//====================================================//

/**
 * @brief The PV (Principal Variation) table. Indexed by ply (from root of search)
 * and the moves in the PV.
 */
bit_move search::PV[MAX_PLY][MAX_PV_SIZE] = {};

/**
 * @brief An array containing for each ply the length of the PV at this ply.
 */
int search::PV_SIZE[MAX_DEPTH] = {};


bit_move search::killers[2][MAX_PV_SIZE] = {};
int search::history[2][64][64] = { {{0}} };
int search::DEPTH = 0;
int search::SELDEPTH = 0;
int search::prev_pv_size = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> search::ENDTIME = {};
std::chrono::time_point<std::chrono::high_resolution_clock> search::STARTTIME = {};
bool search::stop_now = false;

movelist search::moves[256] = {};
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

/**
 * @brief Performs an iterative deepening PVS alpha-beta search.
 *
 * @param b board representation
 * @param depth depth to search to
 * @param quiet flag indicating wether search info should be logged to stdout
 * @return Nodes searched
 */
int search::search_iterative_deepening(bitboard* b, int depth, bool quiet)
{
	stop_now = false;
	int prev_score = 0;
	bit_move best_move = bit_move();
	int i;
	int score = 0;
	int TOTAL_NODES = 0;
	prev_pv_size = 0;
	//==========================================================//
	//
	// iterate from depth 1 to n
	//
	//==========================================================//

	STARTTIME = std::chrono::high_resolution_clock::now();

	for (i = 1; i <= depth; i++)
	{
		NODES_SEARCHED = 0;
		QNODES_SEARCHED = 0;
		DEPTH = i;
		SELDEPTH = i;
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
			//========================================================//
			//
			// aspiration window search
			//
			//========================================================//

			// set aspiration window -- narrow at first
			int prev_alpha = prev_score - 16;
			int prev_beta = prev_score + 16;

			// search with aspiration window
			score = alpha_beta(b, i, prev_alpha, prev_beta, 0);

			// count the number of aspiration window failures
			num_fails = 0;

			// as long as the score is outside the window, reset PV + bestmove and search again
			while ((score <= prev_alpha || score >= prev_beta) && !stop_now)
			{

				// if the score is too low, decrease alpha => increase aspiration window
				prev_alpha = (score <= prev_alpha) ? prev_alpha - (16 << num_fails) : prev_alpha;
				// if the score is too high, increase beta => increase aspiration window
				prev_beta = (score >= prev_beta) ? prev_beta + (16 << num_fails) : prev_beta;
				// research
				score = alpha_beta(b, i, prev_alpha, prev_beta, 0);
				num_fails++;
			}
			prev_score = score;

		}

		//========================================================//
		//
		// print main info to stdout
		//
		// score, depth, seldepth, pv
		//
		//========================================================//

		if (!quiet) {
			std::cout << "info depth " << i << " seldepth " << SELDEPTH << " multipv 1";

			if (std::abs(score) < MATE - MAX_PV_SIZE - 1)
			{
				// print cp score
				std::cout << " score cp " << score << " ";
			}
			else
			{
				// print mate score
				std::cout << " score mate " << utility::sgn(score) * (MATE - std::abs(score) + 1) / 2 << " ";
			}

			best_move = PV[0][0];

			//========================================================//
			//
			// print rest of uci info to stdout
			//
			// nodes, nps
			//
			//========================================================//

			std::cout << "nodes " << NODES_SEARCHED << " ";

			int duration_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - STARTTIME).count();
			// compute nps
			int nps = (duration_ms > 0) ? (int)((double)NODES_SEARCHED / ((double)duration_ms / 1000.0)) : 0;
			std::cout << "nps " << nps << " ";

			std::cout << "tbhits 0 ";
			std::cout << "time " << duration_ms << " ";

			//========================================================//
			//
			// print PV to stdout
			//
			//========================================================//

			gather_and_print_pv(b, score, i);

			std::cout << " num_fails " << (int)num_fails;
			// end the line
			std::cout << std::endl;

		}
		// reset PV-table

		TOTAL_NODES += NODES_SEARCHED;
		if (stop_now) {
			break;
		}
	}

	if (!quiet)
		std::cout << "bestmove " << bit_move::to_string(best_move) << std::endl;
	return TOTAL_NODES;
}

/**
 * @brief Iterates over the PV table and prints the PV to stdout.
 * When the PV contains a null move it should fail gently.
 * When the PV contains an illegal move, it should report this to stdout.
 *
 * It also rehashes the PV nodes into the Transposition Table.
 *
 * @param b board representation
 * @param score the score determined by the search
 * @param curr_depth the depth of the search at the point of calling this
 */
void search::gather_and_print_pv(bitboard* b, int score, int curr_depth) {
	bool root_side = b->side_to_move;
	int j;

	if (PV_SIZE[0] != 0) {
		std::cout << "pv ";

		for (j = 0; j < PV_SIZE[0]; j++)
		{
			bit_move m = PV[0][j];
			// if move is nullmove, break
			if (m.move == 0)
			{
				break;
			}
			// if move is illegal, break
			if (!b->is_legal<false>(&m) && j < curr_depth)
			{
				std::cout << "illegal move in PV! " << bit_move::to_string(m) << std::endl;
				break;
			}
			// record PV in transposition table
			// if score is NOT mate score
			if (std::abs(score) < 90000) {
				tt.set(b->zobrist_key, curr_depth - j, j, (b->side_to_move == root_side) ? score : -score, tt_entry::EXACT, m);
			}
			// if score is mate score, scores need to reflect distance to mate as they are stored in the transposition table
			else {
				int ply_summand = utility::sgn(score) * j;
				int score_corr = score + ply_summand;
				tt.set(b->zobrist_key, curr_depth - j, j, (b->side_to_move == root_side) ? score_corr : -score_corr, tt_entry::EXACT, m);
			}
			b->make_move(&m);
			std::cout << bit_move::to_string(search::PV[0][j]) << " ";
		}
		// unmake PV moves to restore board state
		for (; j > 0; j--)
		{
			b->unmake_move();
		}
	}
	prev_pv_size = PV_SIZE[0];

	clear_PV();

}

/**
 * @brief Performs a quiescence search (searching only captures) until no captures are possible anymore.
 * When there is a check, all moves -- not only captures -- are searched.
 *
 * @param b the board representation
 * @param alpha lower-bound for alpha-beta pruning
 * @param beta upper-boud for alpha-beta pruning
 * @param ply the ply from the root of the search tree
 * @return score of the position
 */
int search::quiescence(bitboard* b, int alpha, int beta, int ply)
{
	/*bit_move hash_move = bit_move();
	int hash_score = tt.probe_qsearch(b->zobrist_key, ply, alpha, beta, &hash_move);
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
	/*int big_delta = 950;
	bool promotion_possible = false;
	if (b->side_to_move) {
		promotion_possible = b->pawns_before_back_rank<true>();
	}
	else {
		promotion_possible = b->pawns_before_back_rank<false>();
	}
	if (promotion_possible) {
		big_delta += 750;
	}
	if (stand_pat + big_delta < alpha) {
		return alpha;
	}*/

	if (stand_pat > alpha)
	{
		alpha = stand_pat;
	}

	int num_legal = 0;
	moves[ply].size = 0;
	if (is_check)
	{
		movegen::generate_all_pseudo_legal_moves(b, &moves[ply]);
	}
	else
	{
		movegen::generate_all_captures(b, &moves[ply]);
	}

	// uint8_t flag = tt_entry::UPPER_BOUND;
	bit_move best_move = bit_move();

	int size = moves[ply].size;
	score_moves(&moves[ply], b->side_to_move);
	for (int i = 0; i < size; i++)
	{
		fetch_next_move(&moves[ply], i);
		bit_move m = moves[ply].moves[i].m;
		if (b->is_legal<true>(&m))
		{
			num_legal++;
			int delta = alpha - stand_pat - 250;
			if (m.get_flags() >= 4 && evaluator::piece_values[0][b->types[m.get_target()]] < delta)
			{
				continue;
			}

			b->make_move(&m);
			NODES_SEARCHED++;
			QNODES_SEARCHED++;

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
				// flag = tt_entry::EXACT;
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

/**
 * @brief Initializes the values for the Late-Move-Reduction table (LMR table).
 *
 * The LMR table is indexed by depth first and move_number second.
 */
void search::init_lmr()
{
	for (int d = 0; d < 256; d++)
	{
		for (int m = 0; m < 256; m++)
		{
			lmr[d][m] = (int)(1.25 + log(d) * log(m) * 100 / 267);
		}
	}
}

/**
 * @brief Implementation of alpha-beta pruning function. Searches until time has
 * elapsed or until given depth has been reached.
 *
 * @param b the board representation in order to make moves and evaluate positions
 * @param depth the maximum depth to search to
 * @param alpha lower bound for alpha-beta pruning
 * @param beta upper bound for alpha-beta pruning
 * @param ply the current ply from the root of the search-tree
 * @return the score of the position
 */
int search::alpha_beta(bitboard* b, int depth, int alpha, int beta, int ply)
{
	// if time is up fail low
	PV_SIZE[ply] = 0;
	if (stop_now)
	{
		return alpha;
	}
	// check if time is up after every 2048 nodes (also check for user input)
	if (NODES_SEARCHED % 2048 == 0)
	{
		communicate();
	}

	//===========================================================//
	//
	// check extension
	//
	//===========================================================//

	// check if we are in check or not
	bool side_to_move = b->side_to_move;
	uint8_t king_pos = b->king_positions[side_to_move];
	bool is_check = b->is_square_attacked(king_pos, !side_to_move);

	// if we are in check, extend search by 1 ply
	depth += is_check;

	//==========================================================//
	//
	// drop into quiescence search
	//
	//==========================================================//

	if (depth == 0)
	{
		return quiescence(b, alpha, beta, ply);
	}

	//==========================================================//
	//
	// check for draw by repetition
	//
	// go back through the game history and check if the zobrist
	// key of the current position appeared already 3 times.
	//
	//==========================================================//
	uint8_t rep_count = 1;
	if (ply != 0) {
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
	}

	// check for draw by 50 move rule
	if (b->fifty_move_rule_counter >= 100)
	{
		return 0;
	}

	bool pv = (beta - alpha) != 1;
	bool zero_window = !pv;
	if (pv) {
		SELDEPTH = std::max(ply, SELDEPTH);
	}
	uint8_t num_legal = 0;
	uint8_t flag = tt_entry::UPPER_BOUND;
	int best_score = -MATE + ply;

	uint8_t LMR = 0;

	bit_move best_move = bit_move();
	bit_move hash_move = bit_move();

	int hash_score = tt.probe(b->zobrist_key, depth, ply, alpha, beta, &hash_move);

	if (hash_score != tt_entry::VAL_UNKNOWN && ply != 0 && zero_window)
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

	//==========================================================//
	//
	// mate dist pruning
	//
	//==========================================================//
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

	if (depth > 2 && !is_check && !has_only_pawns && zero_window && !post_null_move && static_eval >= beta)
	{
		b->make_null_move();
		int R = 2;
		// increase reduction for null move pruning if remaining search-depth is high
		if (depth > 6)
		{
			R++;
		}
		if (depth > 10)
		{
			R++;
		}
		// run a zero window search with reduced depth
		int null_score = -alpha_beta(b, depth - R - 1, -beta, -beta + 1, ply + 1);
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
			NODES_SEARCHED++;

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
				update_PV(&hash_move, ply);
			}
			if (score > best_score)
			{
				best_score = score;
				flag = tt_entry::EXACT;
				best_move = hash_move;
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

	if (!generate_quiets)
	{
		movegen::generate_all_captures(b, &moves[ply]);
		if (moves->size == 0) {
			generate_quiets = true;
			goto generate;
		}
	}
	else
	{
		movegen::generate_all_quiet_moves(b, &moves[ply]);
	}
	score_moves(&moves[ply], side_to_move);

	for (int i = 0; i < moves[ply].size; i++)
	{
		fetch_next_move(&moves[ply], i);
		bit_move m = moves[ply].moves[i].m;

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
			NODES_SEARCHED++;
			int score;
			if (flag == tt_entry::EXACT)
			{
				// search at reduced depth with null window
				score = -alpha_beta(b, depth - 1 - LMR, -alpha - 1, -alpha, ply + 1);
				// if search fails
				if (score > alpha && score < beta)
				{
					// research at full depth with null window
					score = -alpha_beta(b, depth - 1, -alpha - 1, -alpha, ply + 1);
				}
				// if search fails again
				if (score > alpha && score < beta)
				{
					// research at full depth with normal window
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

/**
 * \brief Updates the PV table.
 *
 * The PV table is updated by copying the PV from the previous iteration
 * and adding the new move to the beginning of the PV.
 *
 * \param m The move to add to the PV
 * \param ply The current ply
 */
void search::update_PV(bit_move* m, int ply)
{
	PV[ply][0] = *m;
	for (int i = 0; i < std::min(PV_SIZE[ply + 1], MAX_PV_SIZE - 1); i++)
	{
		PV[ply][i + 1] = PV[ply + 1][i];
	}

	PV_SIZE[ply] = std::min(PV_SIZE[ply + 1] + 1, MAX_PV_SIZE);
}

void search::clear_PV() {
	for (int i = 0; i < MAX_PV_SIZE; i++)
	{
		PV_SIZE[i] = 0;
	}
}

/**
 * @brief Iterates over move list and scores them according to their respective
 * move-ordering scores.
 *
 * Captures are scored with MVV-LVA scoring, non-captures are sorted with scores
 * from the history-heuristic.
 *
 * @param m_l Movelist
 * @param s_l Scorelist
 * @param side_to_move Side to move
 */
void search::score_moves(movelist* m_l, bool side_to_move)
{
	for (int i = 0; i < m_l->size; i++)
	{
		movelist::ML_entry* e = &m_l->moves[i];
		bit_move m = e->m;
		int score;
		if (m.get_flags() >= bit_move::capture)
		{
			score =  mvv_lva[m.get_piece_type()][m.get_captured_type()];
			e->score = score;
		}
		else
		{
			score = history[side_to_move][m.get_origin()][m.get_target()];
			e->score = score;
		}
	}
}

/**
 * @brief Swaps the move with the highest score in the move list to the back of the list. Also swaps the scores.
 * Basically a selection sort.
 *
 * @param m_l Movelist
 * @param s_l Scorelist
 * @param index Index pointing to next element after sorted area
 */
void search::fetch_next_move(movelist* m_l, int index)
{
	// TODO: find bug here, I guess.
	// defining temp variables to store the move and score

	// IDEA: 	* sorted area at the beginning of the array
	// 			* Iterate over array and swap entries such that move with max score is appended to sorted area
	int max_score = m_l->moves[index].score;
	int max_index = index;
	int score;
	for (int i = index + 1; i < m_l->size; i++)
	{
		score = m_l->moves[i].score;
		if (score > max_score)
		{
			max_score = score;
			max_index = i;
		}
	}

	if (max_index != index)
	{
		movelist::ML_entry max_move = m_l->moves[max_index];
		m_l->moves[max_index] = m_l->moves[index];
		m_l->moves[index] = max_move;
	}
}

/**
 * @brief Clears the killer moves.
 */
void search::clear_killers() {
	for (int i = 0; i < MAX_PLY; i++)
	{
		killers[0][i] = bit_move();
		killers[1][i] = bit_move();
	}
}

/**
 * @brief Clears the history table.
 */
void search::clear_history() {
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			history[WHITE][i][j] = 0;
			history[BLACK][i][j] = 0;
		}
	}
}

/**
 * @brief Checks if search time is up and checks for user input that would trigger search abortion.
 */
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
