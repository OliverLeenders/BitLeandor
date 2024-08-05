#include <fstream>
#include <iostream>

#include "movegen.h"
#include "perft.h"
#include "search.h"
#include "tuner.h"

void uci_console();
void bench();
void test_tuner();
void parse_and_make_move(std::vector<std::string> *split, int i);

// value of K in sigmoid function
double K = 1.31f;
std::string VERSION = "2.6.0";

bit_move parse_move(std::vector<std::string> *split, int i);
bitboard b = bitboard();

int main(int argc, char *argv[]) {
    // output compiler name
#ifdef __GNUC__
    std::cout << "Compiled using g++" << std::endl;
#endif // __GNUC__

#ifdef _MSC_VER
    std::cout << "Compiled using MSVC" << std::endl;
#endif // _MSC_VER

    // std::cout << sizeof(std::chrono::time_point<std::chrono::high_resolution_clock>) <<
    // std::endl; initializing attack tables
    attacks::init_attack_tables();

    // generate mafics
    utility::generate_magic_attacks();
    attacks::init_directional_masks();
    // initializing evaluation tables
    weights::init_tables();
    // initializing transposition table
    transposition_table::init_keys();
    search::init_lmr();
    // tuner::init_weights();

    std::string pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // std::string pos = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    // std::string pos = "r1n1k2r/p1pPqpb1/b4np1/4N3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R b KQkq - 0 2";
    // std::string pos = "r3k2r/p2pqpb1/bn2pnp1/2pPN3/1p2P3/P1N2Q1p/1PPBBPPP/R3K2R w KQkq c6 0 2";
    // std::string pos = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    // std::string pos = "4k1n1/8/8/8/8/3r4/4P3/4K1N1 w - - 0 1";
    // std::string pos = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    b.pos_from_fen(pos);

    if (argc > 1) {
        std::string arg_1 = std::string(argv[1]);
        if (arg_1 == "-bench") {
            bench();
            return 0;
        }
    }

    uci_console();

    return 0;
}

/**
 * @brief Provides the functionality of the UCI chess interface.
 *
 * All searches start here, positions can be set up here and search options can be set.
 */
void uci_console() {
    std::cout << "Leandor " << VERSION
              << " by Oliver Leenders. Copyright (C) 2021. Ready to receive commands..."
              << std::endl;
    while (true) {
        std::string line = "";
        std::getline(std::cin, line);
        std::vector<std::string> *split = new std::vector<std::string>;
        utility::split_string(*split, line);
        if (split->size() >= 1) {
            if (split->at(0) == "uci") {
                std::cout << "id name Leandor 2.5.1" << std::endl;
                std::cout << "id author Oliver Leenders" << std::endl;
                std::cout << "uciok" << std::endl;
            }

            else if (split->at(0) == "isready") {
                std::cout << "readyok" << std::endl;
            }

            else if (split->at(0) == "ucinewgame") {
                search::tt.clear();
                evaluator::pawn_tt.clear();
                search::clear_killers();
                search::clear_history();
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
                } else if (split->at(1) == "fen") {
                    std::string fen = "";
                    for (int i = 2; i < 8; i++) {
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
                } else if (split->at(1) == "epd") {
                    std::string epd = "";
                    for (uint16_t i = 2; i < split->size(); i++) {
                        epd += split->at(i) + " ";
                    }
                    b.pos_from_epd_line(epd);
                } else if (split->at(1) == "kiwipete") {
                    b.pos_from_fen(
                        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
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
                        search::ENDTIME = std::chrono::high_resolution_clock().now() +
                                          std::chrono::milliseconds(time);
                        search::search_iterative_deepening(&b, 256, false);
                        search::ENDTIME = {};
                    }
                    if (split->size() >= 9 && split->at(1) == "wtime" && split->at(3) == "btime" &&
                        split->at(5) == "winc" && split->at(7) == "binc") {
                        int w_time = std::stoi((*split)[2]);
                        int b_time = std::stoi((*split)[4]);
                        int w_inc = std::stoi((*split)[6]);
                        int b_inc = std::stoi((*split)[8]);
                        int time = 0;
                        if (b.side_to_move) {
                            time = std::min(b_time, (b_time + 25 * b_inc) / 25);
                        } else {
                            time = std::min(w_time, (w_time + 25 * w_inc) / 25);
                        }
                        search::ENDTIME = std::chrono::high_resolution_clock().now() +
                                          std::chrono::milliseconds(time);
                        search::search_iterative_deepening(&b, 256, false);
                        search::ENDTIME = {};
                    } else if (split->at(1) == "perft") {
                        int depth = std::stoi(split->at(2));
                        perft::run_perft_staged_console(&b, depth);
                    }
                } else if (split->size() == 2 && split->at(1) == "infinite") {
                    search::search_iterative_deepening(&b, 256, false);
                }
            } else if (split->at(0) == "d") {
                b.print_board();
            } else if (split->at(0) == "islegal") {
                if (split->size() == 2) {
                    bit_move m = parse_move(split, 1);
                    if (b.is_legal<false>(&m)) {
                        std::cout << "legal" << std::endl;
                    } else {
                        std::cout << "illegal" << std::endl;
                    }
                }
            } else if (split->at(0) == "remove_loud") {
                // tuner::filter_quiet_positions();
            } else if (split->at(0) == "tune") {
                // tuner::tune(K);
            } else if (split->at(0) == "test") {
                //=======================================================//
                //
                // test tuner
                //
                //=======================================================//
                // std::vector<tuner::ttuple> tuples;
                // tuner::init_coefficients(&b, &tuples);
                // for (tuner::ttuple t : tuples) {
                //     std::cout << "idx: " << (int)t.index << " w: " << (int)t.white_coefficient
                //               << " b: " << (int)t.black_coefficient << std::endl;
                // }
            } else if (split->at(0) == "MSE") {
                // if (split->size() == 2) {
                //     std::ifstream file("quiet-labeled.epd");
                //     std::cout << "Mean-Square Error: "
                //               << tuner::mean_square_error(std::stod(split->at(1)), 1000000, &file)
                //               << std::endl;
                // }
            } else if (split->at(0) == "eval") {
                int score = evaluator::eval(&b);
                std::cout << "material + pst:" << b.PST_score_MG << std::endl;
                std::cout << "material + pst + pawn_structure: " << score << std::endl;
            } else if (split->at(0) == "unmake") {
                if (bit_move::to_string(b.game_history.back().last_move) == "a1a1") {
                    b.unmake_null_move();
                } else {
                    b.unmake_move();
                }
            } else if (split->at(0) == "quit") {
                break;
            } else {
                std::cout << "Unknown command: " << split->at(0) << std::endl;
            }
        }
    }
}

void bench() {
    // read fens_1000.txt
    // split by lines
    // go depth 7-10 or so for each line
    // sum up total number of nodes
    // also benchmark whole thing
    std::string *lines = new std::string[1000];
    std::string line;
    std::ifstream file("fens_1000.txt");
    std::cout << "Reading file \"fens_100.txt\" ... ";
    int i = 0;

    if (file.is_open()) {
        while (std::getline(file, line) && i < 1000) {
            lines[i] = line;
            i++;
        }
    } else {
        std::cout << "Could not find file." << std::endl;
        return;
    }
    {
        {}
    }

    std::cout << "DONE." << std::endl
              << "Searching each FEN position with depth = 7 ... " << std::endl;
    int nodes = 0;
    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();
    for (i = 0; i < 1000; i++) {
        if (i % 50 == 0) {
            std::cout << ".";
        }
        b.pos_from_fen(lines[i]);
        nodes += search::search_iterative_deepening(&b, 7, true);
    }
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    long long d = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << std::endl;
    std::cout << "DONE" << std::endl;

    std::cout << "Total nodes searched: " << nodes << std::endl;
    std::cout << "Total NPS           : " << (int)nodes / d * 1000 << std::endl;

    delete[] lines;
}

bit_move parse_move(std::vector<std::string> *split, int i) {
    uint8_t origin = (std::stoi(split->at(i).substr(1, 2)) - 1) * 8 + (split->at(i).at(0) - 'a');
    uint8_t target = (std::stoi(split->at(i).substr(3, 4)) - 1) * 8 + (split->at(i).at(2) - 'a');

    bool is_capture = b.pieces[target] != EMPTY_PIECE;
    bool is_promotion = b.types[origin] == PAWN && (target < 8 || target >= 56);
    bool is_castle_kingside = b.types[origin] == KING && target == origin + 2;
    bool is_castle_queenside = b.types[origin] == KING && target == origin - 2;
    bool is_double_pawn_push =
        b.types[origin] == PAWN && (target == origin + 16 || target == origin - 16);
    uint8_t promotion_type = 0;
    bool is_ep = (1ULL << target) == b.ep_target_square && b.types[origin] == PAWN;
    uint8_t captured_type = b.types[target];

    if (is_promotion) {
        char promotion_char = split->at(i).at(4);
        if (promotion_char == 'q' || promotion_char == 'Q') {
            promotion_type = QUEEN;
        } else if (promotion_char == 'r' || promotion_char == 'R') {
            promotion_type = ROOK;
        } else if (promotion_char == 'b' || promotion_char == 'B') {
            promotion_type = BISHOP;
        } else if (promotion_char == 'n' || promotion_char == 'N') {
            promotion_type = KNIGHT;
        }
    }

    uint8_t flag = 0;
    if (is_capture && is_promotion) {
        flag = bit_move::knight_capture_promotion + promotion_type - 1;
    } else if (is_ep) {
        flag = bit_move::ep_capture;
        captured_type = PAWN;
    } else if (is_capture) {
        flag = bit_move::capture;
    } else if (is_promotion) {
        flag = bit_move::knight_promotion + promotion_type - 1;
    } else if (is_castle_kingside) {
        flag = bit_move::kingside_castle;
    } else if (is_castle_queenside) {
        flag = bit_move::queenside_castle;
    } else if (is_double_pawn_push) {
        flag = bit_move::double_pawn_push;
    } else {
        flag = bit_move::quiet_move;
    }

    bit_move m = bit_move(origin, target, flag, b.types[origin], captured_type);
    return m;
}

void parse_and_make_move(std::vector<std::string> *split, int i) {
    if (split->at(i) == "a1a1") {
        b.make_null_move();
        return;
    }
    bit_move m = parse_move(split, i);
    b.make_move(&m);
}
