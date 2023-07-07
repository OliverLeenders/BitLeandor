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
                if (b->is_square_attacked(origin, !side_to_move) || b->is_square_attacked(origin - 1, !side_to_move) ||
                    b->is_square_attacked(origin - 2, !side_to_move)) {
                    continue;
                }
            } else if (m.get_flags() == bit_move::kingside_castle) {
                bool side_to_move = b->side_to_move;
                uint8_t origin = m.get_origin();
                if (b->is_square_attacked(origin, !side_to_move) || b->is_square_attacked(origin + 1, !side_to_move) ||
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

uint64_t perft::run_perft_console(bitboard *b, int depth) {
    uint64_t nodes = 0;
    if (depth == 0) {
        return 1;
    } else {
        movelist l;
        movegen::generate_all_pseudo_legal_moves(b, &l);
        std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
        bit_move m;
        for (int i = 0; i < l.size; i++) {
            m = l.moves[i].m;
            if (m.get_flags() == bit_move::queenside_castle) {
                bool side_to_move = b->side_to_move;
                uint8_t origin = m.get_origin();
                if (b->is_square_attacked(origin, !side_to_move) || b->is_square_attacked(origin - 1, !side_to_move) ||
                    b->is_square_attacked(origin - 2, !side_to_move)) {
                    continue;
                }
            } else if (m.get_flags() == bit_move::kingside_castle) {
                bool side_to_move = b->side_to_move;
                uint8_t origin = m.get_origin();
                if (b->is_square_attacked(origin, !side_to_move) || b->is_square_attacked(origin + 1, !side_to_move) ||
                    b->is_square_attacked(origin + 2, !side_to_move)) {
                    continue;
                }
            }
            std::cout << bit_move::to_string(m) << ": ";

            b->make_move(&m);
            evaluator::eval(b);
            bool side_to_move = b->side_to_move;

            if (!b->is_square_attacked(b->king_positions[!side_to_move], side_to_move)) {
                uint64_t curr_nodes = run_perft(b, depth - 1);
                nodes += curr_nodes;
                std::cout << curr_nodes << std::endl;
            }
            b->unmake_move();
            evaluator::eval(b);
        }
        std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
        std::cout << std::endl;
        std::cout << "Total nodes : " << nodes << std::endl;

        long long d = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Duration    : " << d << " ms" << std::endl;
        std::cout << "Perft NPS   : " << nodes / d * 1000 << std::endl;
        std::cout << std::endl;
        return nodes;
    }
}
