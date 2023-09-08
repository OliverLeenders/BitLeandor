/**
 * @file search.cpp
 * @author Oliver Leenders
 * @brief Contains all data declarations and function declarations needed for search.
 * @version 0.1
 * @date 2023-05-04 (last-modified)
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "search.h"

//================================================================================================//
//
// defining static variables
//
//================================================================================================//

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
int search::history[2][64][64] = {{{0}}};

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

int search::lmr[MAX_PV_SIZE][MAX_PV_SIZE] = {{0}};

int search::search_iterative_deepening(bitboard *b, int depth, bool quiet) {
    stop_now = false;
    int prev_score = 0;
    bit_move best_move = bit_move();
    int curr_depth;
    int score = 0;
    int TOTAL_NODES = 0;
    prev_pv_size = 0;
    //============================================================================================//
    //
    // iterate from depth 1 to n
    //
    //============================================================================================//

    STARTTIME = std::chrono::high_resolution_clock::now();

    for (curr_depth = 1; curr_depth <= depth; curr_depth++) {
        NODES_SEARCHED = 0;    // number of nodes spent in a-b-search
        QNODES_SEARCHED = 0;   // number of nodes spent in quiescence
        DEPTH = curr_depth;    // the depth to search to
        SELDEPTH = curr_depth; // the selective search depth
        score = 0;             // score of the position
        int num_fails = 0;     // number of aspiration window fails
        bool valid_score = false;

        // if depth == 1, search without aspiration windows

        if (curr_depth == 1) {
            score = alpha_beta(b, curr_depth, -MATE, MATE, 0);
            prev_score = score;
        } else {
            //====================================================================================//
            //
            // aspiration window search
            //
            //====================================================================================//

            // set aspiration window -- narrow at first
            int prev_alpha = prev_score - 16;
            int prev_beta = prev_score + 16;

            // search with aspiration window
            score = alpha_beta(b, curr_depth, prev_alpha, prev_beta, 0);

            // count the number of aspiration window failures
            num_fails = 0;

            // as long as the score is outside the window, reset PV + bestmove and search again
            while ((score <= prev_alpha || score >= prev_beta) && !stop_now) {

                // if the score is too low, decrease alpha => increase aspiration window
                prev_alpha = (score <= prev_alpha) ? prev_alpha - (16 << num_fails) : prev_alpha;
                // if the score is too high, increase beta => increase aspiration window
                prev_beta = (score >= prev_beta) ? prev_beta + (16 << num_fails) : prev_beta;
                // research
                score = alpha_beta(b, curr_depth, prev_alpha, prev_beta, 0);
                num_fails++;
            }
            // if we abort due to time (or quit) we should only use the score if it lies inside the
            // aspiration window, since it would need to be re-searched otherwise
            if (score > prev_alpha && score < prev_beta) {
                prev_score = score;
                valid_score = true;
            }

        }

        //========================================================================================//
        //
        // print main info to stdout
        //
        // score, depth, seldepth, pv
        //
        //========================================================================================//

        // if we are allowed to log results ...
        if (!quiet) {
            // log results according to UCI specification
            std::cout << "info depth " << curr_depth // log the depth
                      << " seldepth " << SELDEPTH    // log the selective search depth
                      << " multipv 1";               // hardcoded multipv value (not supported)

            // if
            if (std::abs(score) < MATE - MAX_PV_SIZE - 1) {
                // print cp score
                std::cout << " score cp " << score << " ";
            } else {
                // print mate score
                std::cout << " score mate "
                          << utility::sgn(score) * (MATE - std::abs(score) + 1) / 2 << " ";
            }

            //====================================================================================//
            //
            // print rest of uci info to stdout
            //
            // nodes, nps
            //
            //====================================================================================//

            std::cout << "nodes " << NODES_SEARCHED << " ";

            int duration_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                                  std::chrono::high_resolution_clock::now() - STARTTIME)
                                  .count();
            // compute nps
            int nps = (duration_ms > 0)
                          ? (int)((double)NODES_SEARCHED / ((double)duration_ms / 1000.0))
                          : 0;
            std::cout << "nps " << nps << " ";

            std::cout << "tbhits 0 ";
            std::cout << "time " << duration_ms << " ";

            //====================================================================================//
            //
            // print PV to stdout
            //
            //====================================================================================//

            gather_and_print_pv(b, score, curr_depth);

            // end the line
            std::cout << std::endl;
        }
        if (valid_score) {
            best_move = PV[0][0];
        }
        TOTAL_NODES += NODES_SEARCHED;
        if (stop_now) {
            break;
        }
    }

    if (!quiet) {
        std::cout << "bestmove " << bit_move::to_string(best_move) << std::endl;
    }
    return TOTAL_NODES;
}

int search::quiescence(bitboard *b, int alpha, int beta, int ply) {
    uint8_t king_pos = b->king_positions[b->side_to_move];

    // check if we are in check or not
    bool is_check = b->is_square_attacked(king_pos, !b->side_to_move);
    int static_eval;

    // if is in check, static eval should be mate score
    if (is_check) {
        static_eval = -MATE;
    } else {
        static_eval = evaluator::eval(b);
    }

    // if static eval is greater than beta, return beta
    // this assumes that the side to move can does not have to capture or evade multiple attacks.
    if (!is_check && static_eval >= beta) {
        return beta;
    }

    if (static_eval > alpha) {
        alpha = static_eval;
    }

    int num_legal = 0;
    moves[ply].size = 0;
    if (is_check) {
        movegen::generate_all_pseudo_legal_moves(b, &moves[ply]);
    } else {
        movegen::generate_all_captures(b, &moves[ply]);
    }

    bit_move best_move = bit_move();

    int size = moves[ply].size;
    score_moves(&moves[ply], b->side_to_move);
    for (int i = 0; i < size; i++) {
        fetch_next_move(&moves[ply], i);
        bit_move m = moves[ply].moves[i].m;
        if (b->is_legal<true>(&m)) {
            num_legal++;
            int delta = alpha - static_eval - 250;

            //====================================================================================//
            //
            // delta pruning
            //
            //====================================================================================//

            // if (!is_check             // do not prune if in check
            //     && m.get_flags() >= 4 // if move is capture or promotion
            //     && weights::piece_values[0][b->types[m.get_target()]] < delta) { // delta pruning
            //     continue;
            // }

            //====================================================================================//

            b->make_move(&m);
            NODES_SEARCHED++;
            QNODES_SEARCHED++;

            int score = -quiescence(b, -beta, -alpha, ply + 1);

            b->unmake_move();

            if (score >= beta) {
                // tt.set(b->zobrist_key, 0, ply, score, tt_entry::LOWER_BOUND, m);
                return score;
            }

            if (score > alpha) {
                alpha = score;
                // flag = tt_entry::EXACT;
                best_move = m;
            }
        }
    }
    if (is_check && num_legal == 0) {
        return -MATE + ply;
    }
    // tt.set(b->zobrist_key, 0, ply, alpha, flag, best_move);
    return alpha;
}

void search::gather_and_print_pv(bitboard *b, int score, int curr_depth) {
    bool root_side = b->side_to_move;
    // index to iterate forward AND BACKWARD over the PV
    int pv_index;
    if (PV_SIZE[0] != 0) {
        std::cout << "pv ";

        for (pv_index = 0; pv_index < PV_SIZE[0]; pv_index++) {
            bit_move m = PV[0][pv_index];
            // if move is nullmove, break
            if (m.move == 0) {
                break;
            }
            // if move is illegal, break
            if (!b->is_legal<false>(&m) && pv_index < curr_depth) {
                std::cout << "illegal move in PV! " << bit_move::to_string(m) << std::endl;
                break;
            }
            // record PV in transposition table
            // if score is NOT mate score
            if (std::abs(score) < 90000) {
                tt.set(b->zobrist_key, curr_depth - pv_index, pv_index,
                       (b->side_to_move == root_side) ? score : -score, tt_entry::EXACT, m);
            }
            // if score is mate score, scores need to reflect distance to mate as they are stored in
            // the transposition table
            else {
                tt.set(b->zobrist_key, curr_depth - pv_index, pv_index,
                       (b->side_to_move == root_side) ? MATE : -MATE, tt_entry::EXACT, m);
            }
            b->make_move(&m);
            std::cout << bit_move::to_string(search::PV[0][pv_index]) << " ";
        }
        // iterate backward to unmake PV moves and to restore board state
        for (; pv_index > 0; pv_index--) {
            b->unmake_move();
        }
    }
    prev_pv_size = PV_SIZE[0];

    clear_PV();
}

/**
 * @brief Initializes the values for the Late-Move-Reduction table (LMR table).
 *
 * The LMR table is indexed by depth first and move_number second.
 */
void search::init_lmr() {
    for (int d = 0; d < MAX_PLY; d++) {
        for (int m = 0; m < MAX_PLY; m++) {
            lmr[d][m] = (int)(1.25 + log(d) * log(m) * 100 / 267);
        }
    }
}

int search::alpha_beta(bitboard *b, int depth, int alpha, int beta, int ply) {
    // check if time is up after every 2048 nodes (also check for user input)
    if (NODES_SEARCHED % 2048 == 0) {
        communicate();
    }
    // if time is up fail low
    if (stop_now) {
        return alpha;
    }
    PV_SIZE[ply] = 0;

    //============================================================================================//
    //
    // check extension
    //
    //============================================================================================//

    // check if we are in check or not

    bool side_to_move = b->side_to_move;
    uint8_t king_pos = b->king_positions[side_to_move];
    bool is_check = b->is_square_attacked(king_pos, !side_to_move);

    // if we are in check, extend search by 1 ply
    depth += is_check;

    //============================================================================================//
    //
    // check for draw by 50 move rule
    //
    //============================================================================================//

    if (b->fifty_move_rule_counter >= 100) {
        return 0;
    }

    //============================================================================================//
    //
    // check for draw by repetition
    //
    // go back through the game history and check if the zobrist
    // key of the current position appeared already 3 times.
    //
    //============================================================================================//

    uint8_t rep_count = 1;
    if (ply != 0) {
        for (size_t i = b->game_history.size(); i > 0; i--) {
            if (b->game_history[i - 1].last_move.move == 0 ||
                b->game_history[i - 1].last_move.get_flags() != bit_move::quiet_move) {
                break;
            }
            if (b->game_history[i - 1].z_hash == b->zobrist_key) {
                rep_count++;
            }
            if (rep_count == 3) {
                return 0;
            }
        }
    }

    //============================================================================================//
    //
    // drop into quiescence search
    //
    //============================================================================================//

    if (depth == 0) {
        return quiescence(b, alpha, beta, ply);
    }

    //============================================================================================//
    //
    // define important variables (for pruning, reduction, PVS, and general search)
    //
    //============================================================================================//

    // a PV-node should be a node which has not been created by an explicit zero-window search.
    // However this would add more necessary logic (inheriting this from a parent [according to
    // NanoPixel]).
    bool pv = (beta - alpha) != 1;
    bool zero_window = !pv;
    // the selective depth should only be increased if the node is a PV-node
    if (pv) {
        SELDEPTH = std::max(ply, SELDEPTH);
    }
    uint8_t num_legal = 0;
    uint8_t flag = tt_entry::UPPER_BOUND;
    int best_score = -MATE + ply;

    uint8_t LMR = 0;

    //============================================================================================//
    //
    // probe transposition table
    //
    //============================================================================================//

    bit_move best_move = bit_move();
    bit_move hash_move = bit_move();

    int hash_score = tt.probe(b->zobrist_key, depth, ply, alpha, beta, &hash_move);

    bit_move non_generated_move = hash_move;

    // if we got a hit in the TT and we are neither at root nor find a score inside the a-b-window
    // (important for avoiding pv-shortening), we can return the score found
    if (hash_score != tt_entry::VAL_UNKNOWN && ply != 0 /*&& zero_window*/ &&
        (hash_score <= alpha || hash_score >= beta)) {
        // return hash_score;
    }

    int static_eval = evaluator::eval(b);

    bool post_null_move = b->game_history.size() > 0 && b->game_history.back().last_move.move == 0;
    bool has_only_pawns =
        b->occupancy[side_to_move] == (b->bbs[PAWN][side_to_move] | b->bbs[KING][side_to_move]);

    //============================================================================================//
    //
    // mate dist pruning
    //
    //============================================================================================//

    {
        int mating_value = MATE - ply;
        if (mating_value < beta) {
            beta = mating_value;
            if (alpha >= mating_value) {
                return mating_value;
            }
        }

        mating_value = -MATE + ply;

        if (mating_value > alpha) {
            alpha = mating_value;
            if (beta <= mating_value) {
                return mating_value;
            }
        }
    }

    //============================================================================================//
    //
    // Null Move Pruning:
    //
    // IDEA: Pass move to the opponent and search with reduced depth. If the returned score still
    //       fails high, the position is assumed to be so good that it fails high as well. In the
    //       endgame were Zugzwang can happen, this should be skipped.
    //
    //============================================================================================//

    // if (depth > 2 && !is_check && !has_only_pawns /*&& zero_window*/ && !post_null_move &&
    //     static_eval >= beta) {
    //     b->make_null_move();
    //     // run a zero window search with reduced depth
    //     int null_score = -alpha_beta(b, depth - 2 - 1, -beta, -beta + 1, ply + 1);
    //     b->unmake_null_move();
    //     if (null_score >= beta) {
    //         return beta;
    //     }
    // }

    //============================================================================================//
    //
    // Staged Move Generation
    //
    //  Move ordering is as follows:
    //      1. Hash-Move
    //      2. Captures        (MVD-LVA)
    //      3. 2 Killer Moves
    //      4. Quiet Moves     (History Heuristic)
    //
    //============================================================================================//

    uint8_t step = 0;
    uint8_t num_quiets = 0;
    bool generate_quiets = false;

non_generated_moves:
    // if a hash move has been found or the non_generated_move is a killer move
    if (hash_score != transposition_table::VAL_UNKNOWN || step > 0) {
        if (b->is_legal<false>(&non_generated_move)) {
            num_legal++;
            num_quiets += non_generated_move.get_flags() < bit_move::capture;

            b->make_move(&non_generated_move);
            NODES_SEARCHED++;

            int score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);
            b->unmake_move();
            if (score >= beta) {
                tt.set(b->zobrist_key, depth, ply, beta, tt_entry::LOWER_BOUND, non_generated_move);
                return score;
            }
            if (score > alpha) {
                alpha = score;
                update_PV(&non_generated_move, ply);
            }
            if (score > best_score) {
                best_score = score;
                flag = tt_entry::EXACT;
                best_move = non_generated_move;
            }
        }
    }

generate:
    // if we have already searched all non-generated moves, generate captures and later quiet moves
    if (generate_quiets && step < 2) {
        non_generated_move = killers[step][ply];
        step++;
        goto non_generated_moves;
    }

    moves[ply].size = 0;

    if (!generate_quiets) {
        movegen::generate_all_captures(b, &moves[ply]);
        if (moves->size == 0) {
            generate_quiets = true;
            goto generate;
        }
    } else {
        movegen::generate_all_quiet_moves(b, &moves[ply]);
    }
    // score all moves in the movelist so they can be fetched in order later
    score_moves(&moves[ply], side_to_move);
    for (int i = 0; i < moves[ply].size; i++) {
        fetch_next_move(&moves[ply], i);

        bit_move m = moves[ply].moves[i].m;
        if (m.move == hash_move.move || m.move == killers[0][ply].move ||
            m.move == killers[1][ply].move) {
            continue;
        }

        if (b->is_legal<true>(&m)) {
            num_legal++;
            uint8_t move_flags = m.get_flags();
            bool is_quiet_move = move_flags < bit_move::capture;
            num_quiets += is_quiet_move;
            if (is_quiet_move && !is_check && zero_window) {
                // some pruning can go here
                num_quiets++;
            }
            LMR = 0 + (is_quiet_move && (num_legal > 1)) * (lmr[depth - 1][num_legal]);
            b->make_move(&m);
            NODES_SEARCHED++;
            int score;

            score = -alpha_beta(b, depth - 1, -beta, -alpha, ply + 1);

            // unmake the move on the board
            b->unmake_move();
            // evaluate the result of the search after the move
            if (score >= beta) {
                if (m.get_flags() < bit_move::capture) {
                    history[b->side_to_move][m.get_origin()][m.get_target()] += ply * ply;

                    killers[1][ply] = killers[0][ply];
                    killers[0][ply] = m;
                }
                // if the node is a PV-node, store the score in the transposition table
                tt.set(b->zobrist_key, depth, ply, score, tt_entry::LOWER_BOUND, m);
                return score;
            }
            if (score > best_score) {
                best_score = score;
                best_move = m;
                flag = tt_entry::EXACT;
            }
            if (score > alpha) {
                alpha = score;
                update_PV(&m, ply);
            }
        }
    }
    if (!generate_quiets) {
        generate_quiets = true;
        goto generate;
    }

    if (num_legal == 0 && is_check) {
        best_score = -MATE + ply;
        flag = tt_entry::EXACT;
    } else if (num_legal == 0) {
        best_score = 0;
    }
    if (pv) {
        tt.set(b->zobrist_key, depth, ply, best_score, flag, best_move);
    }
    return best_score;
}

void search::update_PV(bit_move *m, int ply) {
    // the first move of the PV at the current should be the move passed as an argument to this
    // function.
    PV[ply][0] = *m;
    // the rest of the moves should be copied from the PV starting at the next ply ...
    for (int i = 0; i < std::min(PV_SIZE[ply + 1], MAX_PV_SIZE - 1); i++) {
        PV[ply][i + 1] = PV[ply + 1][i];
    }
    // finally, the size of the updated pv should be computed
    PV_SIZE[ply] = std::min(PV_SIZE[ply + 1] + 1, MAX_PV_SIZE);
}

void search::clear_PV() {
    for (int i = 0; i < MAX_PV_SIZE; i++) {
        PV_SIZE[i] = 0;
    }
}

void search::score_moves(movelist *m_l, bool side_to_move) {
    for (int i = 0; i < m_l->size; i++) {
        movelist::ML_entry *e = &m_l->moves[i];
        bit_move m = e->m;
        if (m.get_flags() >= bit_move::capture) {
            e->score = mvv_lva[m.get_piece_type()][m.get_captured_type()] + CAPTURE_SCORE;
        } else {
            e->score = history[side_to_move][m.get_origin()][m.get_target()];
        }
    }
}

void search::print_move_list(movelist *m_l) {
    for (int i = 0; i < m_l->size; i++) {
        std::cout << bit_move::to_string(m_l->moves[i].m) << " -- score: " << m_l->moves[i].score
                  << std::endl;
    }
}

void search::fetch_next_move(movelist *m_l, int index) {
    // TODO: find bug here, I guess.
    // defining temp variables to store the move and score

    // IDEA: 	* sorted area at the beginning of the array
    // 			* Iterate over array and swap entries such that move with max score is
    // appended to sorted area
    int max_score = m_l->moves[index].score;
    int max_index = index;
    int score;
    for (int i = index + 1; i < m_l->size; i++) {
        score = m_l->moves[i].score;
        if (score > max_score) {
            max_score = score;
            max_index = i;
        }
    }
    // swap move with best score to front
    if (max_index != index) {
        movelist::ML_entry max_move = m_l->moves[max_index];
        m_l->moves[max_index] = m_l->moves[index];
        m_l->moves[index] = max_move;
    }
}

void search::clear_killers() {
    for (int i = 0; i < MAX_PLY; i++) {
        killers[0][i] = bit_move();
        killers[1][i] = bit_move();
    }
}

void search::clear_history() {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            history[WHITE][i][j] = 0;
            history[BLACK][i][j] = 0;
        }
    }
}

void search::communicate() {
    if (ENDTIME.time_since_epoch().count() != 0LL &&
        std::chrono::high_resolution_clock().now().time_since_epoch().count() >
            ENDTIME.time_since_epoch().count()) {
        stop_now = true;
        return;
    }
    if (input_waiting()) {
        std::string line = "";
        std::getline(std::cin, line);
        if (line != "") {
            std::vector<std::string> *split = new std::vector<std::string>;
            utility::split_string(split, line);
            if (split->at(0) == "stop") {
                stop_now = true;
                return;
            }
        }
    }
}