#pragma once

#include "tt_entry.h"
#include "utility.h"

class transposition_table {
  public:
    static const int VAL_UNKNOWN = 200000;
    transposition_table(int size);
    transposition_table();
    int size = 16777216;
    static void init_keys();
    void clear();
    tt_entry *table = new tt_entry[16777216];
    static uint64_t piece_keys[12][64];
    static uint64_t castling_keys[4];
    static uint64_t en_passant_keys[8];
    static uint64_t side_key;
    /**
     * @brief Stores an entry in the transposition table. Will always replace existing entries.
     *
     * @param key the zobrist-key of the position
     * @param depth depth of the search from the position
     * @param ply ply from root
     * @param eval evaluation of the position
     * @param flag type of the entry
     * @param move best move from the position
     */
    void set(uint64_t key, int depth, int ply, int eval, int flag, bit_move move);
    /**
     * @brief Attempts to retrieve the score of a position from the transposition table. If the
     * position is not found, returns VAL_UNKNOWN.
     *
     * @param key zobrist key of the position
     * @param depth depth of the search from the position
     * @param ply ply from root
     * @param alpha alpha
     * @param beta beta
     * @param m pointer to the move to be returned
     * @return int score of the position
     */
    int probe(uint64_t key, int depth, int ply, int alpha, int beta, bit_move *m);
    int probe_qsearch(uint64_t key, int ply, int alpha, int beta, bit_move *m);
    ~transposition_table();
};