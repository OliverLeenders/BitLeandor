#include <iostream>
#include <chrono>

#include "bitboard_util.h"
#include "bitboard.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include "transposition_table.h"
#include <fstream>

void uci_console();
void bench();
void parse_and_make_move(std::vector<std::string>* split, int i);
bitboard b = bitboard();

int main() {
	// initializing attack tables
	attacks::init_attack_tables();

	// generate mafics
	utility::generate_magic_attacks();
	// initializing evaluation tables
	evaluator::init_tables();
	// initializing transposition table
	transposition_table::init_keys();
	search::init_lmr();


	std::string pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
	// std::string pos = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
	// std::string pos = "r1n1k2r/p1pPqpb1/b4np1/4N3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 2";
	// std::string pos = "r3k2r/p2pqpb1/bn2pnp1/2pPN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R w KQkq c6 0 2";
	// std::string pos = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
	// std::string pos = "4k1n1/8/8/8/8/3r4/4P3/4K1N1 w - - 0 1";
	//std::string pos = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
	b.pos_from_fen(pos);

	uci_console();

	return 0;
}

void uci_console() {
	std::cout << "Leandor 2.0 by Oliver Leenders. Copyright (C) 2021. Ready to receive commands..." << std::endl;
	while (true) {
		std::string line = "";
		std::getline(std::cin, line);
		std::vector<std::string>* split = new std::vector<std::string>;
		utility::split_string(split, line);
		if (split->size() >= 1) {
			if (split->at(0) == "uci") {
				std::cout << "id name Leandor 2.0" << std::endl;
				std::cout << "id author Oliver Leenders" << std::endl;
				std::cout << "uciok" << std::endl;
			}

			else if (split->at(0) == "isready") {
				std::cout << "readyok" << std::endl;
			}

			else if (split->at(0) == "ucinewgame") {
				// do nothing
			}

			else if (split->at(0) == "position") {
				if (split->at(1) == "startpos") {
					b.pos_from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
					if (split->size() >= 3) {
						if (split->at(2) == "moves") {
							for (size_t i = 3; i < split->size(); i++) {
								parse_and_make_move(split, (int)i);
							}
						}

					}
				}
				else if (split->at(1) == "fen") {
					std::string fen = "";
					for (int i = 2; i < 6; i++) {
						fen += split->at(i) + " ";
					}
					b.pos_from_fen(fen);
					if (split->size() >= 9) {
						if (split->at(8) == "moves") {
							for (size_t i = 9; i < split->size(); i++) {
								parse_and_make_move(split, (int)i);
							}
						}
					}
				}
				else if (split->at(1) == "kiwipete") {
					b.pos_from_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
					if (split->size() >= 3) {
						if (split->at(2) == "moves") {
							for (size_t i = 3; i < split->size(); i++) {
								parse_and_make_move(split, (int)i);
							}
						}

					}
				}
			}

			else if (split->at(0) == "bench") {
				bench();
			}

			else if (split->at(0) == "go") {
				if (split->size() >= 3) {
					if (split->at(1) == "depth") {
						int depth = std::stoi(split->at(2));
						search::search_iterative_deepening(&b, depth, false);
					}
					if (split->at(1) == "movetime") {
						int time = std::stoi(split->at(2));
						search::ENDTIME = std::chrono::high_resolution_clock().now() + std::chrono::milliseconds(time);
						search::search_iterative_deepening(&b, 256, false);
						search::ENDTIME = {};
					}
					if (split->size() >= 9 && split->at(1) == "wtime" && split->at(3) == "btime" && split->at(5) == "winc" && split->at(7) == "binc") {
						int w_time = std::stoi((*split)[2]);
						int b_time = std::stoi((*split)[4]);
						int w_inc = std::stoi((*split)[6]);
						int b_inc = std::stoi((*split)[8]);
						int time = 0;
						if (b.side_to_move) {
							time = std::min(b_time, (b_time + 25 * b_inc) / 25);
						}
						else {
							time = std::min(w_time, (w_time + 25 * w_inc) / 25);
						}
						search::ENDTIME = std::chrono::high_resolution_clock().now() + std::chrono::milliseconds(time);
						search::search_iterative_deepening(&b, 256, false);
						search::ENDTIME = {};
					}
					else if (split->at(1) == "perft") {
						int depth = std::stoi(split->at(2));
						perft::run_perft_console(&b, depth);
					}
				}
				else if (split->size() == 2 && split->at(1) == "infinite") {
					search::search_iterative_deepening(&b, 256, false);
				}
			}
			else if (split->at(0) == "d") {
				b.print_board();
			}
			else if (split->at(0) == "unmake") {
				if (bit_move::to_string(b.game_history.back().last_move) == "a1a1") {
					b.unmake_null_move();
				}
				else {
					b.unmake_move();
				}
			}
			else if (split->at(0) == "quit") {
				break;
			}
			else {
				std::cout << "Unknown command: " << split->at(0) << std::endl;
			}

		}
	}
}

void bench()
{
	// read fens_1000.txt
	// split by lines
	// go depth 7-10 or so for each line
	// sum up total number of nodes
	// also benchmark whole thing
	std::string* lines = new std::string[1000];
	std::string line;
	std::ifstream file("fens_1000.txt");
	std::cout << "Reading file \"fens_100.txt\" ... ";
	int i = 0;
	while (std::getline(file, line) && i < 1000) {
		lines[i] = line;
		i++;
	}
	std::cout << "DONE." << std::endl << "Searching each FEN position with depth = 7 ... " << std::endl;
	int nodes = 0;
	for (i = 0; i < 1000; i++) {
		if (i % 50 == 0) {
			std::cout << ".";
		}
		b.pos_from_fen(lines[i]);
		nodes += search::search_iterative_deepening(&b, 7, true);
	}
	std::cout << std::endl;
	std::cout << "DONE" << std::endl;

	std::cout << "Total nodes searched: " << nodes << std::endl;

	delete[] lines;

}

void parse_and_make_move(std::vector<std::string>* split, int i)
{
	if (split->at(i) == "a1a1") {
		b.make_null_move();
		return;
	}
	uint8_t origin = (std::stoi(split->at(i).substr(1, 2)) - 1) * 8 + (split->at(i).at(0) - 'a');
	uint8_t target = (std::stoi(split->at(i).substr(3, 4)) - 1) * 8 + (split->at(i).at(2) - 'a');

	bool is_capture = b.pieces[target] != EMPTY_PIECE;
	bool is_promotion = b.types[origin] == PAWN && (target < 8 || target >= 56);
	bool is_castle_kingside = b.types[origin] == KING && target == origin + 2;
	bool is_castle_queenside = b.types[origin] == KING && target == origin - 2;
	bool is_double_pawn_push = b.types[origin] == PAWN && (target == origin + 16 || target == origin - 16);
	uint8_t promotion_type = 0;
	bool is_ep = (1ULL << target) == b.ep_target_square && b.types[origin] == PAWN;
	uint8_t captured_type = b.types[target];

	if (is_promotion) {
		char promotion_char = split->at(i).at(4);
		if (promotion_char == 'q') {
			promotion_type = QUEEN;
		}
		else if (promotion_char == 'r') {
			promotion_type = ROOK;
		}
		else if (promotion_char == 'b') {
			promotion_type = BISHOP;
		}
		else if (promotion_char == 'n') {
			promotion_type = KNIGHT;
		}
	}

	uint8_t flag = 0;
	if (is_capture && is_promotion) {
		flag = bit_move::knight_capture_promotion + promotion_type - 1;
	}
	else if (is_ep) {
		flag = bit_move::ep_capture;
		captured_type = PAWN;
	}
	else if (is_capture) {
		flag = bit_move::capture;
	}
	else if (is_promotion) {
		flag = bit_move::knight_promotion + promotion_type - 1;
	}
	else if (is_castle_kingside) {
		flag = bit_move::kingside_castle;
	}
	else if (is_castle_queenside) {
		flag = bit_move::queenside_castle;
	}
	else if (is_double_pawn_push) {
		flag = bit_move::double_pawn_push;
	}
	else {
		flag = bit_move::quiet_move;
	}

	bit_move m = bit_move(origin, target, flag, b.types[origin], captured_type);
	b.make_move(&m);
}

