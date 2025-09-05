#include "transposition_table.h"

uint64_t transposition_table::piece_keys[12][64] = {{0}};
uint64_t transposition_table::castling_keys[4] = {0};
uint64_t transposition_table::en_passant_keys[8] = {0};
uint64_t transposition_table::side_key = 0ULL;

transposition_table::transposition_table(int size) {
    this->table = std::make_unique<tt_entry[]>(size);
    this->size = size;
}

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

uint64_t transposition_table::set_size(int size_in_mb) {
    uint64_t upper_bound = std::min(size_in_mb, MAX_SIZE_MB) * 1024 * 1024 / sizeof(tt_entry);
    // set actual size to the largest power of two smaller than upper_bound
    uint64_t size = 1;
    while ((size << 1) < upper_bound) {
        size <<= 1;
    }
    size = std::max(1ULL, size);
    this->table = std::make_unique<tt_entry[]>(size);
    this->size = size;
    return size;
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
    int index = key % size;
    // if is mate score, adjust score
    if (score >= MATE - 256) {
        score += ply;
    }
    if (score <= -MATE + 256) {
        score -= ply;
    }
    table[index].score = score;
    table[index].key = key;
    table[index].depth = depth;
    table[index].type = flag;
    table[index].hash_move = move;
    // std::cout << "set: score " << table[index].score << " key " << table[index].key << " d "
    //            << (int)table[index].depth << " f " << (int)table[index].type << " m "
    //           << bit_move::to_string(table[index].hash_move) << "\n";
}

int transposition_table::probe(uint64_t key, int depth, int ply, int alpha, int beta, bit_move *m) {
    tt_entry entry = this->table[key % this->size];
    if (entry.key == key) {
        m->move = entry.hash_move.move;
        if (entry.depth >= depth) {
            if (entry.type == tt_entry::EXACT ||
                (entry.type == tt_entry::UPPER_BOUND && entry.score <= alpha) ||
                (entry.type == tt_entry::LOWER_BOUND && entry.score >= beta)) {
                int score = entry.score;
                // if is mate score, adjust score
                if (score >= MATE - 256) {
                    score -= ply;
                } else if (score <= -MATE + 256) {
                    score += ply;
                }
                return score;
            }
        }
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
    }
    return VAL_UNKNOWN;
}

transposition_table::~transposition_table() {}
