#pragma once
#include <ctype.h>

#include <cstdint>
#include <string>
#include <vector>

#include "bit_move.h"
#include "board_state.h"
#include "constants.h"
#include "transposition_table.h"
#include "utility.h"
#include "weights.h"

class bitboard {
  public:
    bitboard();
    // piece bitboards
    uint64_t bbs[6][2] = {{0ULL, 0ULL}, {0ULL, 0ULL}, {0ULL, 0}, {0ULL, 0ULL}, {0ULL, 0ULL}, {0ULL, 0ULL}};
    // occupancy bitboards
    uint64_t occupancy[3] = {0ULL};

    // bitboard of the en-passant target square
    uint64_t ep_target_square = 0ULL;

    uint64_t zobrist_key = 0ULL;
    uint64_t pawn_hash_key = 0ULL;

    // scores for incremental update
    int16_t PST_score_MG = 0;
    int16_t PST_score_EG = 0;

    uint16_t fifty_move_rule_counter = 0;
    uint16_t full_move_clock = 1;

    uint8_t types[64] = {EMPTY};
    uint8_t pieces[64] = {EMPTY_PIECE};

    // king positions (stored for faster access)
    uint8_t king_positions[2] = {0U, 0U};
    char castling_rights = 0;

    // side to move switch
    bool side_to_move = 0;

    // castling right encoding
    // clang-format off
    enum castling_rights_e {
        w_kingside  = 1, // 0001
        w_queenside = 2, // 0010
        b_kingside  = 4, // 0100
        b_queenside = 8  // 1000
    };
    // clang-format on

    void pos_from_fen(std::string fen);
    void pos_from_epd_line(std::string epd_l);
    std::string pos_to_fen();
    int char_to_rank(char c);
    int char_to_file(char c);
    int piece_to_char(uint8_t piece);
    bool is_square_attacked(int square, bool side_to_move);

    /**
     * @brief Checks whether there are pawns of the side_to_move ready to
     * promote
     *
     * @tparam @c side_to_move side to move
     * @return @c true if there are pawns threatening promotion,
     * @return @c false if there are no pawns threatening promotion
     */
    template <bool side_to_move> bool pawns_before_back_rank() {
        uint64_t rank = (side_to_move) ? second_rank : seventh_rank;
        uint64_t promotion_candidates = bbs[PAWN][side_to_move] & rank;
        return promotion_candidates != 0ULL;
    }

    /**
     * @brief Prints a ASCII-art image of the current board state.
     */
    void print_board();

    /**
     * @brief Destroy the bitboard object.
     */
    ~bitboard();
    bool is_sane();

    // templated is legal function needs to be in header-file for some stupid
    // c++ reason...
    template <bool was_generated> bool is_legal(bit_move *m) {
        // clang-format off
        const uint8_t flags          = m->get_flags();
        const uint8_t origin 		 = m->get_origin();
        const uint8_t target 		 = m->get_target();
        const uint8_t type   		 = m->get_piece_type();
        const uint8_t captured_type  = m->get_captured_type();
        const uint8_t piece  		 = (side_to_move) ? type + BLACK_PAWN : type;
        const uint8_t captured_piece = (side_to_move) ? captured_type     : captured_type + BLACK_PAWN;
        // clang-format on

        // if the move was not generated we need to check
        // the availability of a piece which is able to move
        // to that square (and more) stil incomplete -- does
        // not check vacancy of sliding piece rays

        if (!was_generated) {
            uint64_t target_bb = 1ULL << target;
            // some conditions to fail early
            if (origin == target) {
                return false;
            }
            if (type == EMPTY) {
                return false;
            }
            // if the piece is not on the origin square or origin square is
            // empty
            if (piece != pieces[origin] || pieces[origin] == EMPTY_PIECE) {
                return false;
            }
            // if the move is a quiet move and the target square is not empty
            if (flags == bit_move::quiet_move) {
                if (pieces[target] != EMPTY_PIECE) {
                    return false;
                }
            } else if (flags == bit_move::capture || flags >= bit_move::knight_capture_promotion) {
                if (captured_piece != pieces[target]) { // this one is critical
                    return false;
                }
            } else if (flags == bit_move::ep_capture) {
                if (target_bb != ep_target_square) {
                    return false;
                }
            } else if (flags == bit_move::double_pawn_push) {
                if (pieces[target] != EMPTY_PIECE || pieces[target - ((side_to_move) ? -8 : 8)] != EMPTY_PIECE) {
                    return false;
                }
            } else if (flags == bit_move::kingside_castle) {
                // if we have no castling right
                if (!(this->castling_rights & ((side_to_move) ? b_kingside : w_kingside))) {
                    return false;
                }
                // if the squares are occupied
                if (pieces[origin + 1] != EMPTY_PIECE || pieces[origin + 2] != EMPTY_PIECE) {
                    return false;
                }
            } else if (flags == bit_move::queenside_castle) {
                // if we have no castling right
                if (!(this->castling_rights & ((side_to_move) ? b_queenside : w_queenside))) {
                    return false;
                }
                // if the squares are occupied
                if (pieces[origin - 1] != EMPTY_PIECE || pieces[origin - 2] != EMPTY_PIECE ||
                    pieces[origin - 3] != EMPTY_PIECE) {
                    return false;
                }
            }
            if (types[origin] == KNIGHT) {
                if ((target_bb & attacks::knight_attacks[origin]) == 0ULL) {
                    return false;
                }
            }
            if (types[origin] == KING && flags != bit_move::kingside_castle && flags != bit_move::queenside_castle) {
                if ((target_bb & attacks::king_attacks[origin]) == 0ULL) {
                    return false;
                }
            }
            if (types[origin] == PAWN) {
                if (captured_type != EMPTY) {
                    if ((target_bb & attacks::pawn_attacks[side_to_move][origin]) == 0ULL) {
                        return false;
                    }
                } else {
                    if (side_to_move == WHITE) {
                        if ((target - origin != 16 && origin < 16) && target - origin != 8) {
                            return false;
                        }
                    } else {
                        if ((origin - target != 16 && origin > 47) && origin - target != 8) {
                            return false;
                        }
                    }
                }
            }
            if (types[origin] >= BISHOP && types[origin] <= QUEEN) {
                // return false if origin and target are not on the same
                // diagonal, file or rank
                if (types[origin] == BISHOP) { // Bishop
                    if ((target_bb & attacks::diagonal_masks[origin]) == 0ULL) {
                        return false;
                    }
                }

                if (types[origin] == ROOK) { // Rook
                    if (((1ULL << target) & attacks::horizontal_vertical_masks[origin]) == 0ULL) {
                        return false;
                    }
                }

                if (types[origin] == QUEEN) {
                    if (((1ULL << target) &
                         (attacks::diagonal_masks[origin] | attacks::horizontal_vertical_masks[origin])) == 0ULL) {
                        return false;
                    }
                }

                // return false if there is a piece between origin and target
                if ((attacks::squares_between[origin][target] & occupancy[2]) != 0ULL) {
                    return false;
                }
            }
        }
        if (flags == bit_move::queenside_castle) {
            if (is_square_attacked(origin, !side_to_move) || is_square_attacked(origin - 1, !side_to_move) ||
                is_square_attacked(origin - 2, !side_to_move)) {
                return false;
            }
        } else if (flags == bit_move::kingside_castle) {
            if (is_square_attacked(origin, !side_to_move) || is_square_attacked(origin + 1, !side_to_move) ||
                is_square_attacked(origin + 2, !side_to_move)) {
                return false;
            }
        }
        make_move(m);
        // uint8_t king_pos = BitScanForward64(bbs[KING][!side_to_move]);
        if (is_square_attacked(king_positions[!side_to_move], side_to_move)) {
            unmake_move();
            return false;
        }
        unmake_move();
        return true;
    }

    void make_move(bit_move *m);
    void unmake_move();
    /**
     * @brief Places a piece on the board, updating the zobrist key accordingly.
     *
     * @param piece piece to be placed
     * @param target position where piece should be placed
     *
     * @tparam update_zobrist whether this action should update the zobrist key
     * @tparam update_score whether this action should update the PST_score
     */
    template <bool, bool> void place_piece(uint8_t piece, uint8_t target);
    
    template <bool, bool> void unset_piece(uint8_t target);
    template <bool, bool> void replace_piece(uint8_t piece, uint8_t target);
    void make_null_move();
    void unmake_null_move();
    uint8_t piece_type_from_index(unsigned long i);
    std::vector<board_state> game_history = {};
    // std::list<std::pair<std::string, std::string>> last_100_moves = {};
};
