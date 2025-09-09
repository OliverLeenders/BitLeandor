#include "evaluator.h"

int evaluator::pawn_tt_hits = 0;
int evaluator::pawn_tt_misses = 0;

pawn_transposition_table evaluator::pawn_tt = pawn_transposition_table(4096 << 2);

int evaluator::eval(bitboard *b) {
    int midgame_score = 0;
    int endgame_score = 0;

    int phase = b->game_phase;

    // check if position is a drawn endgame
    if (is_draw(b)) {
        return 0;
    }

    //============================================================================================//
    //
    // evaluate material + piece square tables
    //
    //============================================================================================//

    midgame_score = b->PST_score_MG;
    endgame_score = b->PST_score_EG;


    // // bishop mobility
    // uint64_t occ = b->occupancy[2];
    // uint64_t bishops = b->bbs[BISHOP][WHITE] | b->bbs[BISHOP][BLACK];
    // int index;
    // int factor;
    // while (bishops != 0ULL) {
    //     index = BitScanForward64(bishops);
    //     factor = (b->pieces[index] < BLACK_PAWN) ? 1 : -1;
    //     midgame_score += (int)((float)PopCount64(attacks::get_bishop_attacks(index, occ)) * weights::mobility_factors[BISHOP] * factor);
    //     endgame_score += (int)((float)PopCount64(attacks::get_bishop_attacks(index, occ)) * weights::mobility_factors[BISHOP] * factor);
    //     bishops &= reset_lsb(bishops);
    // }

    // // rook mobility
    // uint64_t rooks = b->bbs[ROOK][WHITE] | b->bbs[ROOK][BLACK];
    // while (rooks != 0ULL) {
    //     index = BitScanForward64(rooks);
    //     factor = (b->pieces[index] < BLACK_PAWN) ? 1 : -1;
    //     midgame_score += (int)((float)PopCount64(attacks::get_rook_attacks(index, occ)) * weights::mobility_factors[ROOK] * factor);
    //     endgame_score += (int)((float)PopCount64(attacks::get_rook_attacks(index, occ)) * weights::mobility_factors[ROOK] * factor);
    //     rooks &= reset_lsb(rooks);
    // }


    // if there are pawns on the board, evaluate the pawn structure
    // if ((b->bbs[PAWN][WHITE] | b->bbs[PAWN][BLACK])) {
    //     eval_pawn_structure(b, &midgame_score, &endgame_score);
    // }

    //============================================================================================//
    //
    // combine endgame and midgame scores
    //
    //============================================================================================//

    int score = (midgame_score * phase + endgame_score * (weights::game_phase_sum - phase)) /
                weights::game_phase_sum;

    // scores need to be from the perspective of the side to move
    // => so they are flipped if necessary
    return (b->side_to_move) ? -score : score;
}

void evaluator::eval_pawn_structure(bitboard *b, int *mg_score, int *eg_score) {
    int hash_score = pawn_tt.probe_mg(b->pawn_hash_key);
    if (hash_score != transposition_table::VAL_UNKNOWN) {
        (*mg_score) += hash_score;
        hash_score = pawn_tt.probe_eg(b->pawn_hash_key);
        (*eg_score) += hash_score;
        pawn_tt_hits++;
        return;
    }
    pawn_tt_misses++;

    int p_mg_score = 0;
    int p_eg_score = 0;

    uint64_t w_pawns = b->bbs[PAWN][WHITE];
    uint64_t b_pawns = b->bbs[PAWN][BLACK];

    uint8_t pawn_islands[2] = {0U};

    //============================================================================================//
    //
    // double pawn penalty + half open files
    //
    // computes a penalty for double pawns, and checks which files are
    // half open at the same time
    //
    //============================================================================================//
    uint64_t file;
    for (int i = 0; i < 8; i++) {
        file = files[i];
        if ((w_pawns & file) != 0ULL) { // at least one white pawn on file
            p_mg_score -=
                ((int)PopCount64(w_pawns & file) - 1) * weights::double_pawn_penalty[MIDGAME][i];
            p_eg_score -=
                ((int)PopCount64(w_pawns & file) - 1) * weights::double_pawn_penalty[ENDGAME][i];
            pawn_islands[WHITE] |= 1 << i;
        }
        if ((b_pawns & file) != 0ULL) { // at least one black pawn on file
            p_mg_score +=
                ((int)PopCount64(b_pawns & file) - 1) * weights::double_pawn_penalty[MIDGAME][i];
            p_eg_score +=
                ((int)PopCount64(b_pawns & file) - 1) * weights::double_pawn_penalty[ENDGAME][i];
            pawn_islands[BLACK] |= 1 << i;
        }
    }

    int8_t white_islands = pawn_islands[WHITE];
    int8_t black_islands = pawn_islands[BLACK];

    white_islands = white_islands & (white_islands >> 1);
    black_islands = black_islands & (black_islands >> 1);

    white_islands = PopCount8(white_islands) - 1;
    black_islands = PopCount8(black_islands) - 1;

    // penalty for high number of pawn islands
    p_mg_score -= (white_islands - black_islands) * weights::pawn_island_penalty[MIDGAME];
    p_eg_score -= (white_islands - black_islands) * weights::pawn_island_penalty[ENDGAME];

    //============================================================================================//
    //
    // passed pawn bonus
    //
    //============================================================================================//
    uint8_t index;
    while (w_pawns != 0ULL) {
        index = BitScanForward64(w_pawns);
        if (((patterns::front_spans[WHITE][index] | patterns::attack_front_spans[WHITE][index]) &
             b_pawns) == 0ULL) {
            p_mg_score += weights::pp_bonus[WHITE][MIDGAME][index];
            p_eg_score += weights::pp_bonus[WHITE][ENDGAME][index];
        }
        w_pawns = reset_lsb(w_pawns);
    }
    // it is necessary to restore the pawn bitboard for what follows
    w_pawns = b->bbs[PAWN][WHITE];
    while (b_pawns != 0ULL) {
        index = BitScanForward64(b_pawns);
        if (((patterns::front_spans[BLACK][index] | patterns::attack_front_spans[BLACK][index]) &
             w_pawns) == 0ULL) {
            p_mg_score += weights::pp_bonus[BLACK][MIDGAME][index];
            p_eg_score += weights::pp_bonus[BLACK][ENDGAME][index];
        }
        b_pawns &= reset_lsb(b_pawns);
    }

    //============================================================================================//
    //
    // isolated pawn penalty
    //
    //============================================================================================//

    w_pawns = b->bbs[PAWN][WHITE];
    b_pawns = b->bbs[PAWN][WHITE];

    for (int i = 0; i < 8; i++) {
        uint64_t file = files[i];
        uint64_t neighbour = patterns::neighbour_files[i];
        if ((w_pawns & file) && !(w_pawns & neighbour)) { // white pawn is isolated
            p_mg_score -= weights::isolated_pawn_penalties[MIDGAME][i];
            p_eg_score -= weights::isolated_pawn_penalties[ENDGAME][i];
        }
        if ((b_pawns & file) && !(w_pawns & neighbour)) { // black pawn is isolated
            p_mg_score += weights::isolated_pawn_penalties[MIDGAME][i];
            p_eg_score += weights::isolated_pawn_penalties[ENDGAME][i];
        }
    }

    //============================================================================================//
    //
    // king safety (pawn shields)
    //
    //============================================================================================//

    // if white king is on one of the safe squares on the kingside
    if (b->bbs[KING][WHITE] & weights::safe_king_positions[WHITE][KINGSIDE]) {
        uint64_t pawn_shield = weights::pawn_shields[WHITE][KINGSIDE] & b->bbs[PAWN][WHITE];
        p_mg_score -= (3 - PopCount64(pawn_shield)) * weights::pawn_shield_penalty_ks;
    }

    // if white king is on one of the safe squares on the queenside
    if (b->bbs[KING][WHITE] & weights::safe_king_positions[WHITE][QUEENSIDE]) {
        uint64_t pawn_shield = weights::pawn_shields[WHITE][QUEENSIDE] & b->bbs[PAWN][WHITE];
        p_mg_score -= (3 - PopCount64(pawn_shield)) * weights::pawn_shield_penalty_qs;
    }

    // if black king is on one of the safe squares on the kingside
    if (b->bbs[KING][BLACK] & weights::safe_king_positions[BLACK][KINGSIDE]) {
        uint64_t pawn_shield = weights::pawn_shields[BLACK][KINGSIDE] & b->bbs[PAWN][BLACK];
        p_mg_score += (3 - PopCount64(pawn_shield)) * weights::pawn_shield_penalty_ks;
    }

    // if black king is on one of the safe squares on the queenside
    if (b->bbs[KING][BLACK] & weights::safe_king_positions[BLACK][QUEENSIDE]) {
        uint64_t pawn_shield = weights::pawn_shields[BLACK][QUEENSIDE] & b->bbs[PAWN][BLACK];
        p_mg_score += (3 - PopCount64(pawn_shield)) * weights::pawn_shield_penalty_qs;
    }

    //============================================================================================//
    //
    // king safety (virtual mobility)
    //
    //============================================================================================//

    // white king safety
    uint64_t mobility_bb = attacks::get_rook_attacks(b->king_positions[WHITE],
                                                     b->occupancy[WHITE] | b->bbs[PAWN][BLACK]) |
                           attacks::get_bishop_attacks(b->king_positions[WHITE],
                                                       b->occupancy[WHITE] | b->bbs[PAWN][BLACK]);

    int16_t w_king_mobility = PopCount64(mobility_bb);

    mobility_bb = attacks::get_rook_attacks(b->king_positions[BLACK],
                                            b->occupancy[BLACK] | b->bbs[PAWN][WHITE]) |
                  attacks::get_bishop_attacks(b->king_positions[BLACK],
                                              b->occupancy[BLACK] | b->bbs[PAWN][WHITE]);
    // black king safety
    int16_t b_king_mobility = PopCount64(mobility_bb);

    p_mg_score += (b_king_mobility - w_king_mobility);

    //============================================================================================//
    //
    // store entry in TT
    //
    //============================================================================================//

    pawn_tt.set_entry(b->pawn_hash_key, p_mg_score, p_eg_score);

    (*mg_score) += p_mg_score;
    (*eg_score) += p_eg_score;

    return;
}

bool evaluator::is_draw(bitboard *b) {
    if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK])) {
        return true;
    } else if ((b->bbs[PAWN][WHITE] | b->bbs[PAWN][BLACK]) == 0ULL) {

        //========================================================================================//
        //
        //  return 0 if endgame is drawn
        //		basically check whether the pieces suffice to mate
        //		or whether one side will not be able to win
        //
        //========================================================================================//

        // check KR-KR
        if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[ROOK][WHITE] |
                                b->bbs[ROOK][BLACK]) &&
            PopCount64(b->bbs[ROOK][WHITE]) == 1 && PopCount64(b->bbs[ROOK][BLACK]) == 1) {
            return true;
        }
        // check KNN-K, KN-K, KNN-KN
        if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[KNIGHT][WHITE] |
                                b->bbs[KNIGHT][BLACK]) &&
            ((PopCount64(b->bbs[KNIGHT][WHITE]) <= 2 && PopCount64(b->bbs[KNIGHT][BLACK]) <= 1) ||
             (PopCount64(b->bbs[KNIGHT][WHITE]) <= 1 && PopCount64(b->bbs[KNIGHT][BLACK]) <= 2))) {
            return true;
        }
        // check KN-KB
        if (b->occupancy[2] ==
                (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[KNIGHT][WHITE] |
                 b->bbs[KNIGHT][BLACK] | b->bbs[BISHOP][WHITE] | b->bbs[BISHOP][BLACK]) &&
            ( // white has knight, black has bishop
                (PopCount64(b->bbs[KNIGHT][WHITE]) == 1 && PopCount64(b->bbs[KNIGHT][BLACK]) == 0 &&
                 PopCount64(b->bbs[BISHOP][BLACK]) == 1 && PopCount64(b->bbs[BISHOP][WHITE]) == 0)
                // black has knight, white has bishop
                || (b->bbs[KNIGHT][WHITE] == 0ULL && PopCount64(b->bbs[KNIGHT][BLACK]) == 1 &&
                    b->bbs[BISHOP][BLACK] == 0ULL && PopCount64(b->bbs[BISHOP][WHITE]) == 1))) {
            return true;
        }
        // check KB-K
        if (b->occupancy[2] == (b->bbs[KING][WHITE] | b->bbs[KING][BLACK] | b->bbs[BISHOP][WHITE] |
                                b->bbs[BISHOP][BLACK]) &&
            ((PopCount64(b->bbs[BISHOP][WHITE]) == 1 && b->bbs[BISHOP][BLACK] == 0ULL) ||
             (b->bbs[BISHOP][WHITE] == 0ULL && PopCount64(b->bbs[BISHOP][BLACK]) == 1))) {
            return true;
        }
    }
    return false;
}

int evaluator::tuner_eval(bitboard *b) {
    int midgame_score = 0;
    int endgame_score = 0;

    uint8_t phase = 0;

    uint8_t index = 0;
    uint8_t type = 0;
    uint64_t occ = b->occupancy[2];
    uint8_t piece = EMPTY;

    if (is_draw(b)) {
        return 0;
    }

    //============================================================================================//
    //
    // evaluate material + piece square tables
    //
    //============================================================================================//

    while (occ != 0ULL) {
        index = BitScanForward64(occ);
        piece = b->pieces[index];
        type = b->types[index];
        int factor = (piece < BLACK_PAWN) ? 1 : -1;
        midgame_score += weights::tuner_piece_values[MIDGAME][type] * factor +
                         weights::piece_square_tables[MIDGAME][piece][index];
        endgame_score += weights::tuner_piece_values[ENDGAME][type] * factor +
                         weights::piece_square_tables[ENDGAME][piece][index];
        phase += weights::game_phase_values[b->types[index]];
        occ &= occ - 1;
    }

    if ((b->bbs[PAWN][WHITE] | b->bbs[PAWN][BLACK])) {
        eval_pawn_structure(b, &midgame_score, &endgame_score);
    }

    //============================================================================================//
    //
    // combine endgame and midgame scores
    //
    //============================================================================================//

    int score = (midgame_score * phase + (endgame_score) * (weights::game_phase_sum - phase)) /
                weights::game_phase_sum;
    return score;
}

/*
Tuning finished with error: 0.0679752
Piece values:
117, 391, 435, 652, 1288, 0,
108, 396, 418, 634, 1258, 0,
Game Phase Values:
0, 41, 40, 60, 101, 0,
Pawn shield penalty (kingside): 13
Pawn shield penalty (queenside): 28
Double pawn penalty:
29, 22, 31, 10, 6, 35, 48, 24,
67, 35, 53, 16, 21, 48, 31, 49,
*/
