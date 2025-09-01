#include "perft.h"

uint64_t perft::run_perft(bitboard *b, int depth) {
    uint64_t nodes = 0;
    if (depth == 0) {
        return 1;
    } else {
        movelist l;
        movegen::generate_all_pseudo_legal_moves(b, &l);
        for (int i = 0; i < l.size; i++) {
            bit_move m = l.moves[i].m;
            if (m.get_flags() == bit_move::queenside_castle) {
                bool side_to_move = b->side_to_move;
                uint8_t origin = m.get_origin();
                if (b->is_square_attacked(origin, !side_to_move) ||
                    b->is_square_attacked(origin - 1, !side_to_move) ||
                    b->is_square_attacked(origin - 2, !side_to_move)) {
                    continue;
                }
            } else if (m.get_flags() == bit_move::kingside_castle) {
                bool side_to_move = b->side_to_move;
                uint8_t origin = m.get_origin();
                if (b->is_square_attacked(origin, !side_to_move) ||
                    b->is_square_attacked(origin + 1, !side_to_move) ||
                    b->is_square_attacked(origin + 2, !side_to_move)) {
                    continue;
                }
            }
            b->make_move(&m);
            evaluator::eval(b);

            bool side_to_move = b->side_to_move;

            if (!b->is_square_attacked(b->king_positions[!side_to_move], side_to_move)) {
                // std::cout << bit_move::to_string(m) << std::endl;
                nodes += run_perft(b, depth - 1);
            }
            b->unmake_move();
            evaluator::eval(b);
        }
        return nodes;
    }
}

uint64_t perft::brute_force_perft(bitboard *b) {
    std::cout << "starting..." << std::endl;
    uint64_t count = 0ULL;
    for (int origin = 0; origin < 64; origin++) {
        for (int target = 0; target < 64; target++) {
            for (uint8_t type = Types::PAWN; type <= Types::EMPTY; type++) {
                for (uint8_t captured_type = Types::PAWN; captured_type <= Types::EMPTY; captured_type++) {
                    for (uint8_t flag : bit_move::flag_list) {
                        bit_move m = bit_move(origin, target, flag, type, captured_type);

                        if (b->is_legal<false>(&m)) {
                            std::cout << bit_move::to_long_string(m) << std::endl;
                            // b->print_board();
                            count++;
                        }
                    }
                }
            }
        }
    }
    std::cout << "Computed BFP: " << count << " moves" << std::endl;
    return count;
}

uint64_t perft::run_perft_console(bitboard *b, int depth) {
    uint64_t nodes = 0;
    if (depth == 0) {
        return 1;
    } else {
        movelist l;
        movegen::generate_all_pseudo_legal_moves(b, &l);
        std::chrono::time_point<std::chrono::high_resolution_clock> start =
            std::chrono::high_resolution_clock::now();
        bit_move m;
        for (int i = 0; i < l.size; i++) {
            m = l.moves[i].m;
            if (m.get_flags() == bit_move::queenside_castle) {
                bool side_to_move = b->side_to_move;
                uint8_t origin = m.get_origin();
                if (b->is_square_attacked(origin, !side_to_move) ||
                    b->is_square_attacked(origin - 1, !side_to_move) ||
                    b->is_square_attacked(origin - 2, !side_to_move)) {
                    continue;
                }
            } else if (m.get_flags() == bit_move::kingside_castle) {
                bool side_to_move = b->side_to_move;
                uint8_t origin = m.get_origin();
                if (b->is_square_attacked(origin, !side_to_move) ||
                    b->is_square_attacked(origin + 1, !side_to_move) ||
                    b->is_square_attacked(origin + 2, !side_to_move)) {
                    continue;
                }
            }
            std::cout << bit_move::to_string(m) << ": ";

            b->make_move(&m);
            bool side_to_move = b->side_to_move;

            if (!b->is_square_attacked(b->king_positions[!side_to_move], side_to_move)) {
                uint64_t curr_nodes = run_perft(b, depth - 1);
                nodes += curr_nodes;
                std::cout << curr_nodes << std::endl;
            }
            b->unmake_move();
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> end =
            std::chrono::high_resolution_clock::now();
        std::cout << std::endl;
        std::cout << "Total nodes : " << nodes << std::endl;

        long long d = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Duration    : " << d << " ms" << std::endl;
        std::cout << "Perft NPS   : " << nodes / d * 1000 << std::endl;
        std::cout << std::endl;
        return nodes;
    }
}

uint64_t perft::run_perft_staged(bitboard *b, int depth, int ply) {
    if (depth == 0) {
        return 1;
    }
    uint64_t nodes = 0ULL;
    bit_move m = bit_move();

    movegen::init_movegen(bit_move(), ply, b->side_to_move, movegen::CAPTURES);
    while (movegen::provide_next_move(b, &m)) {
        b->make_move(&m);
        nodes += run_perft_staged(b, depth - 1, ply + 1);
        b->unmake_move();
    }
    movegen::reset_movegen(ply, !b->side_to_move);

    return nodes;
}

uint64_t perft::run_perft_staged_console(bitboard *b, int depth) {
    if (depth == 0) {
        return 1;
    }
    uint64_t nodes = 0ULL;
    std::chrono::time_point<std::chrono::high_resolution_clock> start =
        std::chrono::high_resolution_clock::now();
    bit_move m = bit_move();
    movegen::init_movegen(bit_move(), 0, b->side_to_move, movegen::CAPTURES);

    while (movegen::provide_next_move(b, &m)) {
        std::cout << bit_move::to_string(m) << ": ";

        b->make_move(&m);
        bool side_to_move = b->side_to_move;

        uint64_t curr_nodes = run_perft_staged(b, depth - 1, 1);
        nodes += curr_nodes;
        std::cout << curr_nodes << std::endl;

        b->unmake_move();
    }
    movegen::reset_movegen(-1, !b->side_to_move);
    std::chrono::time_point<std::chrono::high_resolution_clock> end =
        std::chrono::high_resolution_clock::now();
    std::cout << std::endl;
    std::cout << "Total nodes : " << nodes << std::endl;

    long long d = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Duration    : " << d << " ms" << std::endl;
    if (d != 0.0) {
        std::cout << "Perft NPS   : " << nodes / d * 1000 << std::endl;
    } else {
        std::cout << "Perft NPS   : INF" << std::endl;
    }
    std::cout << std::endl;

    return 0ULL;
}
