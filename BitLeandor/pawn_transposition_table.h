#pragma once

#include "transposition_table.h"
#include "pawn_tt_entry.h"
class pawn_transposition_table
{
public:
    pawn_transposition_table(int size);
    ~pawn_transposition_table();
    int size = 0;
    pawn_tt_entry* table = new pawn_tt_entry[size];
    void set_entry(uint64_t key, int mg_score, int eg_score);
    void clear();
    int probe(uint64_t key, int phase);
    int probe_mg(uint64_t key);
    int probe_eg(uint64_t key);
};
