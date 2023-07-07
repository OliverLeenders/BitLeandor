/**
 * @file evaluator.h
 * @author Oliver Leenders (oliver.leenders@gmx.net)
 * @brief Contains all information and functions to evaluate chess positions.
 * @version 0.1
 * @date 2023-05-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include "bitboard.h"
#include "patterns.h"
#include "pawn_transposition_table.h"
#include "pawn_tt_entry.h"
#include "transposition_table.h"
#include "weights.h"

const int EVAL_ERROR = MATE + 666;

/**
 * @brief Contains all functions relating to evaluating a position.
 */
class evaluator {
  public:
    /**
     * @brief Statically evaluates the position. Takes into account material, piece placement, pawn
     * structure and king safety.
     *
     * @param b board representation
     * @return the score of the position
     */
    static int eval(bitboard *b);
    /**
     * @brief Evaluates the pawn structure of a position and updates the values of the midgame score
     * and endgame score accordingly
     *
     * @param b board representation
     * @param mg_score pointer to mg_score
     * @param eg_score pointer to eg_score
     */
    static void eval_pawn_structure(bitboard *b, int *mg_score, int *eg_score);
    static bool is_draw(bitboard *b);

    static int tuner_eval(bitboard *b);

    // pawn structure transposition table
    static pawn_transposition_table pawn_tt;

    static int pawn_tt_hits;
    static int pawn_tt_misses;
};
