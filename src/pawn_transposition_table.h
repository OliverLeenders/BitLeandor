#pragma once

#include "pawn_tt_entry.h"
#include "transposition_table.h"

class pawn_transposition_table {
  public:
    pawn_transposition_table(int size);
    ~pawn_transposition_table();
    uint32_t size = 0;
    pawn_tt_entry *table = new pawn_tt_entry[size];
    /**
     * @brief Sets the midgame score and endgame score of the value mapped to by the key.
     *
     * @param key the hash key
     * @param mg_score midgame score
     * @param eg_score endgame score
     */
    void set_entry(uint64_t key, int mg_score, int eg_score);
    void clear();
    int probe(uint64_t key, int phase);
    int probe_mg(uint64_t key);
    int probe_eg(uint64_t key);
};
