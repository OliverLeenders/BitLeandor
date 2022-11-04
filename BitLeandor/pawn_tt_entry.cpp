#include "pawn_tt_entry.h"

pawn_tt_entry::pawn_tt_entry() {
    this->eg_score = VAL_UNKNOWN;
    this->mg_score = VAL_UNKNOWN;
    this->key = 0ULL;
}

pawn_tt_entry::pawn_tt_entry(uint64_t set_key, int set_eg_score, int set_mg_score) {
    this->eg_score = set_eg_score;
    this->mg_score = set_mg_score;
    this->key = set_key;
}

pawn_tt_entry::~pawn_tt_entry() {
}