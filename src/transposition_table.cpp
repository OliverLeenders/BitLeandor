#include "transposition_table.h"

uint64_t transposition_table::piece_keys[12][64] = {{0}};
uint64_t transposition_table::castling_keys[4] = {0};
uint64_t transposition_table::en_passant_keys[8] = {0};
uint64_t transposition_table::side_key = 0ULL;

transposition_table::transposition_table(int size) {
    delete[] table;
    this->table = new tt_entry[size];
    this->size = size;
}

transposition_table::transposition_table() {}

void transposition_table::clear() {
    for (int i = 0; i < size; i++) {
        // clang-format off
		table[i].key 			= 0;
        table[i].depth 			= 0;
        table[i].score 			= VAL_UNKNOWN;
        table[i].type 			= 0;
        table[i].hash_move.move = 0;
        // clang-format on
    }
}

void transposition_table::init_keys() {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 64; j++) {
            piece_keys[i][j] = utility::pseudo_rand_64_bit_num();
        }
    }

    for (int i = 0; i < 4; i++) {
        castling_keys[i] = utility::pseudo_rand_64_bit_num();
    }

    for (int i = 0; i < 8; i++) {
        en_passant_keys[i] = utility::pseudo_rand_64_bit_num();
    }

    side_key = utility::pseudo_rand_64_bit_num();
}

void transposition_table::set(uint64_t key, int depth, int ply, int score, int flag,
                              bit_move move) {
    if (std::abs(score) >= MATE - 1000) {
        // upper bound mate scores are weird and should not be stored according to Bruce Moreland.
        if (flag == tt_entry::UPPER_BOUND && score > 0 ||
            flag == tt_entry::LOWER_BOUND && score < 0) {
            return;
        } else {
            flag = tt_entry::LOWER_BOUND;
            score = utility::sgn(score) * (MATE - 1000);
        }
    }
    int index = key % size;
    // clang-format off
	table[index].key       = key;
    table[index].depth     = depth;
    table[index].score     = score;
    table[index].type 	   = flag;
    table[index].hash_move = move;
    // clang-format on
}

int transposition_table::probe(uint64_t key, int depth, int ply, int alpha, int beta, bit_move *m) {
    tt_entry entry = this->table[key % this->size];
    if (entry.key == key) {
        m->move = entry.hash_move.move;
        if (entry.depth >= depth) {
            if (entry.type == tt_entry::EXACT) {
                return entry.score;
            }
            if (entry.type == tt_entry::UPPER_BOUND && entry.score <= alpha) {
                return alpha;
            }
            if (entry.type == tt_entry::LOWER_BOUND && entry.score >= beta) {
                return beta;
            }
        }
        // std::cout << "Hash move: " << bit_move::to_string(pos_hash_entry->hash_move) <<
        // std::endl;
    }

    return VAL_UNKNOWN;
}

int transposition_table::probe_qsearch(uint64_t key, int ply, int alpha, int beta, bit_move *m) {
    tt_entry entry = this->table[key % this->size];
    if (entry.key == key) {
        m->move = entry.hash_move.move;
        if (entry.type == tt_entry::EXACT) {
            return entry.score;
        }
        if (entry.type == tt_entry::UPPER_BOUND && entry.score <= alpha) {
            return alpha;
        }
        if (entry.type == tt_entry::LOWER_BOUND && entry.score >= beta) {
            return beta;
        }
        // std::cout << "Hash move: " << bit_move::to_string(pos_hash_entry->hash_move) <<
        // std::endl;
    }

    return VAL_UNKNOWN;
}

transposition_table::~transposition_table() { delete[] table; }
