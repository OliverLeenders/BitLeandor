#include <iostream>

#include "bitboard_util.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"

int main() {
	// initializing attack tables
	attacks::init_attack_tables();
	// generate mafics
	Utility::generate_magic_attacks();
	evaluator::init_tables();

	std::string pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	// std::string pos = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	// std::string pos = "r3k2r/p2pqpb1/bn2pnp1/2pPN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R w KQkq c6 0 2";
	//std::string pos = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
	// std::string pos = "4k1n1/8/8/8/8/3r4/4P3/4K1N1 w - - 0 1";
	//std::string pos = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
	bitboard board;
	board.pos_from_fen(pos);
	/*movelist l_0;
	movegen::generate_all_pseudo_legal_moves(&board, &l_0);
	board.make_move(&l_0.moves[0]);
	/*movelist l_1;
	movegen::generate_all_pseudo_legal_moves(&board, &l_1);
	board.make_move(&l_1.moves[11]);
	movelist l_2;
	movegen::generate_all_pseudo_legal_moves(&board, &l_2);
	board.make_move(&l_2.moves[1]);*/

	board.print_board();

	//std::cout << perft::run_perft_console(&board, 5) << std::endl;
	std::cout << search::quiescence(&board, -search::MATE, search::MATE, 0) << std::endl;
	std::cout << "eval: " << evaluator::eval(&board) << std::endl;
	std::cout << search::alpha_beta(&board, 7, -search::MATE, search::MATE, 0) << std::endl;
	int pv_index = search::get_pv_index(7);
	for (int i = 0; i < 7; i++) {
		std::cout << bit_move::to_string(search::PV[pv_index + i]) << " ";
	}
	std::cout << std::endl;
	std::cout << "nodes: " << search::NODES_SEARCHED << std::endl;
	//bitboard_util::print_bitboard(attacks::get_bishop_attacks(2, board.pieces[2]));
	board.print_board();
	//bitboard_util::print_bitboard(board.kings[0]);
	return 0;
}