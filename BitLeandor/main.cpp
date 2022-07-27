#include <iostream>

#include "bitboard_util.h"
#include "bitboards.h"
#include "movegen.h"
int main() {
    // initializing attack tables
    attacks::init_attack_tables();
    // generate mafics
    Utility::generate_magic_attacks();

    std::string pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    bitboard board;
    board.pos_from_fen(pos);
    bitboard_util::print_bitboard(board.pawns[0]);
    movelist l;
    movegen::generate_knight_qmoves(board, &l);
    for (int i = 0; i < l.size; i++) {
        std::cout << bit_move::to_string(l.moves[i]) << std::endl;
    }

    return 0;
}