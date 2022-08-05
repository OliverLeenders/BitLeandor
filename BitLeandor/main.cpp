#include <iostream>

#include "bitboard_util.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"

int main() {
	// initializing attack tables
	attacks::init_attack_tables();
	// generate mafics
	Utility::generate_magic_attacks();

	std::string pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	bitboard board;
	board.pos_from_fen(pos);
	movelist l_0;
	//movegen::generate_all_pseudo_legal_moves(&board, &l_0);
	//board.make_move(&l_0.moves[0]);
	// movelist l_1;
	// movegen::generate_all_pseudo_legal_moves(&board, &l_1);
	// board.make_move(&l_1.moves[0]);
	bitboard_util::print_bitboard(board.pieces[2]);
	std::cout <<perft::run_perft_console(&board, 3) << std::endl;
	bitboard_util::print_bitboard(board.pieces[2]);
	return 0;
}