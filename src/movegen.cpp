#include "movegen.h"

movegen::movegen() {}

movegen::~movegen() {}

void movegen::generate_queen_qmoves(bitboard *b, movelist *l) {
    uint64_t queens = b->bbs[QUEEN][b->side_to_move];
    unsigned long origin;
    unsigned long target;
    uint64_t queen_attacks = 0ULL;
    while (queens != 0ULL) {
        origin = BitScanForward64(queens);
        queen_attacks = attacks::get_rook_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
        queen_attacks |= attacks::get_bishop_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
        while (queen_attacks) {
            target = BitScanForward64(queen_attacks);
            l->moves[l->size].m =
                bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, QUEEN, EMPTY);
            l->size++;
            queen_attacks = reset_lsb(queen_attacks);
        }
        queens = reset_lsb(queens);
    }
}

void movegen::generate_queen_cmoves(bitboard *b, movelist *l) {
    uint64_t queens = b->bbs[QUEEN][b->side_to_move];
    unsigned long origin;
    unsigned long target;
    uint64_t queen_attacks = 0ULL;
    uint8_t capture_type = EMPTY;
    while (queens != 0ULL) {
        origin = BitScanForward64(queens);
        queen_attacks =
            attacks::get_rook_attacks(origin, b->occupancy[2]) & (b->occupancy[!b->side_to_move]);
        queen_attacks |=
            attacks::get_bishop_attacks(origin, b->occupancy[2]) & (b->occupancy[!b->side_to_move]);
        while (queen_attacks != 0ULL) {
            target = BitScanForward64(queen_attacks);
            capture_type = b->piece_type_from_index(target);
            l->moves[l->size].m =
                bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, QUEEN, capture_type);
            l->size++;
            queen_attacks = reset_lsb(queen_attacks);
        }
        queens = reset_lsb(queens);
    }
}

void movegen::generate_king_cmoves(bitboard *b, movelist *l) {
    uint64_t opp_pieces = b->occupancy[!b->side_to_move];
    unsigned long origin = b->king_positions[b->side_to_move];
    unsigned long target;
    uint64_t king_attacks = attacks::king_attacks[origin] & opp_pieces;
    uint8_t capture_type = EMPTY;
    while (king_attacks != 0ULL) {
        target = BitScanForward64(king_attacks);
        capture_type = b->piece_type_from_index(target);
        l->moves[l->size].m =
            bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, KING, capture_type);
        l->size++;
        king_attacks = reset_lsb(king_attacks);
    }
}

void movegen::generate_all_captures(bitboard *b, movelist *l) {
    if (b->side_to_move) {
        generate_pawn_cmoves<true>(b, l);
        generate_knight_cmoves<true>(b, l);
        generate_bishop_cmoves<true>(b, l);
        generate_rook_cmoves<true>(b, l);
    } else {
        generate_pawn_cmoves<false>(b, l);
        generate_knight_cmoves<false>(b, l);
        generate_bishop_cmoves<false>(b, l);
        generate_rook_cmoves<false>(b, l);
    }
    generate_queen_cmoves(b, l);
    generate_king_cmoves(b, l);
}

void movegen::generate_all_quiet_moves(bitboard *b, movelist *l) {
    if (b->side_to_move) {
        generate_pawn_qmoves<true>(b, l);
        generate_king_qmoves<true>(b, l);
        generate_knight_qmoves<true>(b, l);
        generate_bishop_qmoves<true>(b, l);
        generate_rook_qmoves<true>(b, l);
    } else {
        generate_pawn_qmoves<false>(b, l);
        generate_king_qmoves<false>(b, l);
        generate_knight_qmoves<false>(b, l);
        generate_bishop_qmoves<false>(b, l);
        generate_rook_qmoves<false>(b, l);
    }
    generate_queen_qmoves(b, l);
}

void movegen::generate_all_pseudo_legal_moves(bitboard *b, movelist *l) {
    generate_all_captures(b, l);
    generate_all_quiet_moves(b, l);
}

//================================================================================================//
//
// State machine
//
//================================================================================================//

uint8_t movegen::state[256] = {HASH_MOVE};
bit_move movegen::hash_move[256] = {bit_move()};
bit_move movegen::killers[256][2] = {{bit_move()}};
int movegen::ply = 0;
bool movegen::side_to_move[256] = {false};
uint8_t movegen::current_move_index[256] = {0};
movelist movegen::MGEN_movelist[256] = {movelist()};
int movegen::history[2][64][64] = {{{0}}};

void movegen::init_movegen(bit_move hash_move, int ply, bool side_to_move, uint8_t state) {
    movegen::ply = ply;
    movegen::hash_move[ply] = hash_move;
    movegen::side_to_move[ply] = side_to_move;
    movegen::current_move_index[ply] = 0;
    movegen::MGEN_movelist[ply].size = 0;
    movegen::state[ply] = state;
}

void movegen::reset_movegen(int ply, int side_to_move) {
    movegen::ply = ply - 1;
    if (ply > 0) {
        movegen::side_to_move[ply] = side_to_move;
    }
}

bool movegen::provide_next_move(bitboard *b, bit_move *m) {
    if (state[ply] == HASH_MOVE) {
        state[ply] = KILLER_1;
        if (b->is_legal<false>(&hash_move[ply])) {
            *m = hash_move[ply];
            return true;
        }
    }
    if (state[ply] == KILLER_1) {
        state[ply] = KILLER_2;
        if (b->is_legal<false>(&killers[ply][0])) {
            *m = killers[ply][0];
            return true;
        }
    }
    if (state[ply] == KILLER_2) {
        state[ply] = CAPTURES;
        if (b->is_legal<false>(&killers[ply][1])) {
            *m = killers[ply][1];
            return true;
        }
    }
    if (state[ply] == CAPTURES) {
        if (current_move_index[ply] == 0) {
            MGEN_movelist[ply].size = 0;
            generate_all_captures(b, &MGEN_movelist[ply]);
            score_moves();
        }
        while (current_move_index[ply] < MGEN_movelist[ply].size) {
            bit_move current_move;
            find_next_move(&current_move);
            current_move_index[ply]++;
            if (current_move != hash_move[ply] && current_move != killers[ply][0] &&
                current_move != killers[ply][1] && b->is_legal<true>(&current_move)) {
                *m = current_move;
                return true;
            }
        }
        state[ply] = QUIET_MOVES;
        current_move_index[ply] = 0;
    }
    if (state[ply] == QUIET_MOVES) {
        if (current_move_index[ply] == 0) {
            MGEN_movelist[ply].size = 0;
            generate_all_quiet_moves(b, &MGEN_movelist[ply]);
        }
        while (current_move_index[ply] < MGEN_movelist[ply].size) {
            bit_move current_move = MGEN_movelist[ply].moves[current_move_index[ply]].m;
            current_move_index[ply]++;
            if (b->is_legal<true>(&current_move)) {
                *m = current_move;
                return true;
            }
        }
    }
    // state should be DONE
    m = nullptr;
    return false;
}

void movegen::score_moves() {
    // if this is called, then the moves have already been generated
    // so a check for hash_move, killer_1, and killer_2 is not necessary (they should have been
    // provided ealier -- they exist as duplicates in the movelist [#TODO: fix this maybe])

    if (state[ply] == CAPTURES) {
        // score captures
        for (int i = 0; i < MGEN_movelist[ply].size; i++) {
            movelist::ML_entry e = MGEN_movelist[ply].moves[i];
            e.score = MVV_LVA[e.m.get_captured_type()][e.m.get_piece_type()];
        }
    } else { // state should be QUIET_MOVES
        // score quiet moves
        for (int i = 0; i < MGEN_movelist[ply].size; i++) {
            movelist::ML_entry e = MGEN_movelist[ply].moves[i];
            e.score = history[side_to_move[ply]][e.m.get_origin()][e.m.get_target()];
        }
    }
}

// TODO: there is a bug
void movegen::find_next_move(bit_move *m) {
    int max_score = MGEN_movelist[ply].moves[current_move_index[ply]].score;
    int max_index = current_move_index[ply];
    int score;
    for (int i = current_move_index[ply] + 1; i < MGEN_movelist[ply].size; i++) {
        score = MGEN_movelist[ply].moves[i].score;
        if (score > max_score) {
            max_score = score;
            max_index = i;
        }
    }
    // swap move with best score to front
    if (max_index != current_move_index[ply]) {
        movelist::ML_entry max_move = MGEN_movelist[ply].moves[max_index];
        MGEN_movelist[ply].moves[max_index] = MGEN_movelist[ply].moves[current_move_index[ply]];
        MGEN_movelist[ply].moves[current_move_index[ply]] = max_move;
    }
    *m = MGEN_movelist[ply].moves[current_move_index[ply]].m;
}