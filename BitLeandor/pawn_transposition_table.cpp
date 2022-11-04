#include "pawn_transposition_table.h"

pawn_transposition_table::pawn_transposition_table(int size) {
    this->size = size;
    delete[] this->table;
    this->table = new pawn_tt_entry[size];
}

pawn_transposition_table::~pawn_transposition_table() {
    delete[] this->table;
}

void pawn_transposition_table::set_entry(uint64_t key, int eg_score, int mg_score) {
    int index = key % size;
    table[index].eg_score = eg_score;
    table[index].mg_score = mg_score;
}

int pawn_transposition_table::probe(uint64_t key, int phase) {
    int index = key % size;
    pawn_tt_entry entry = table[index];
    if (key == entry.key) {
        return (phase * entry.mg_score + (76 - phase) * entry.eg_score) / 76;}
    else {
        return transposition_table::VAL_UNKNOWN;
    }
}