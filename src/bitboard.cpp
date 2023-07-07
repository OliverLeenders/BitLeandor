#include "bitboard.h"

bitboard::bitboard() { this->game_history.reserve(512); }

bitboard::~bitboard() {}

bool bitboard::is_sane() {
    if (bbs[KING][WHITE] == 0ULL || bbs[KING][BLACK] == 0ULL) {
        print_board();
    }
    if (king_positions[WHITE] != BitScanForward64(bbs[KING][WHITE]) ||
        king_positions[BLACK] != BitScanForward64(bbs[KING][BLACK])) {
        print_board();
    }
    uint64_t hash_key = 0ULL;
    for (int i = 0; i < 64; i++) {
        uint8_t piece = pieces[i];
        uint8_t type = types[i];
        if (type != EMPTY) {
            if ((bbs[type][WHITE] & (1ULL << i)) == 0 && (bbs[type][BLACK] & (1ULL << i)) == 0) {
                std::cout << "Type " << ((int)type) << " not in bitboard" << std::endl << i << std::endl;
                print_board();
                return false;
            }
        }
        if (piece != EMPTY_PIECE) {
            hash_key ^= transposition_table::piece_keys[piece][i];
            if ((bbs[piece % BLACK_PAWN][piece >= BLACK_PAWN] & (1ULL << i)) == 0) {
                std::cout << "Piece " << ((int)piece) << " not in bitboard" << std::endl << i << std::endl;
                print_board();
                return false;
            }
        } else {
            if ((occupancy[2] & 1ULL << i) != 0ULL) {
                std::cout << "empty square is occupied" << std::endl;
                print_board();
                return false;
            }
        }
    }
    if (side_to_move) {
        hash_key ^= transposition_table::side_key;
    }
    if (hash_key != this->zobrist_key) {
        // std::cout << "hash key mismatch: correct ->" << hash_key << "
        // incorrect -> " << zobrist_key << std::endl; print_board(); return
        // false;
    }
    return true;
}

void bitboard::pos_from_fen(std::string fen) {
    std::vector<std::string> fen_split;
    utility::split_string(&fen_split, fen);
    this->zobrist_key = 0ULL;
    for (int i = 0; i < 6; i++) {
        this->bbs[i][0] = 0ULL;
        this->bbs[i][1] = 0ULL;
    }

    this->PST_score_MG = 0;
    this->PST_score_EG = 0;

    for (int i = 0; i < 3; i++) {
        this->occupancy[i] = 0ULL;
    }

    this->ep_target_square = -1;
    this->castling_rights = 0;
    this->game_history.clear();

    if (fen_split.size() < 5) {
        std::cout << "fen string too short" << std::endl;
        return;
    }
    std::string fen_pos = fen_split[0];
    for (int i = 0; i < 64; i++) {
        this->types[i] = EMPTY;
        this->pieces[i] = EMPTY_PIECE;
    }
    int fen_pos_index = 0;
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            int board_index = 8 * i + j;
            char c = fen_pos.at(fen_pos_index);
            if (std::isdigit(c)) {
                int digit = c - '0';
                j += digit - 1;
            } else {
                switch (c) {
                case 'P':
                    this->place_piece<true, true>(WHITE_PAWN, board_index);
                    break;
                case 'p':
                    this->place_piece<true, true>(BLACK_PAWN, board_index);
                    break;
                case 'N':
                    this->place_piece<true, true>(WHITE_KNIGHT, board_index);
                    break;
                case 'n':
                    this->place_piece<true, true>(BLACK_KNIGHT, board_index);
                    break;
                case 'B':
                    this->place_piece<true, true>(WHITE_BISHOP, board_index);
                    break;
                case 'b':
                    this->place_piece<true, true>(BLACK_BISHOP, board_index);
                    break;
                case 'R':
                    this->place_piece<true, true>(WHITE_ROOK, board_index);
                    break;
                case 'r':
                    this->place_piece<true, true>(BLACK_ROOK, board_index);
                    break;
                case 'Q':
                    this->place_piece<true, true>(WHITE_QUEEN, board_index);
                    break;
                case 'q':
                    this->place_piece<true, true>(BLACK_QUEEN, board_index);
                    break;
                case 'K':
                    this->place_piece<true, true>(WHITE_KING, board_index);
                    this->king_positions[WHITE] = board_index;
                    break;
                case 'k':
                    this->place_piece<true, true>(BLACK_KING, board_index);
                    this->king_positions[BLACK] = board_index;
                    break;
                default:
                    std::cout << "An error occured while reading the fen -- "
                                 "illegal position string. char at fault: '"
                              << c << "'." << std::endl;
                    break;
                }
            }
            fen_pos_index++;
        }
        fen_pos_index++;
    }

    this->occupancy[0] = this->bbs[PAWN][0] | this->bbs[KNIGHT][0] | this->bbs[BISHOP][0] | this->bbs[ROOK][0] |
                         this->bbs[QUEEN][0] | this->bbs[KING][0];
    this->occupancy[1] = this->bbs[PAWN][1] | this->bbs[KNIGHT][1] | this->bbs[BISHOP][1] | this->bbs[ROOK][1] |
                         this->bbs[QUEEN][1] | this->bbs[KING][1];
    this->occupancy[2] = this->occupancy[0] | this->occupancy[1];

    std::string fen_side_to_move = fen_split[1];
    this->side_to_move = fen_side_to_move == "b";
    if (side_to_move) {
        this->zobrist_key ^= transposition_table::side_key;
    }

    std::string fen_castling_rights = fen_split[2];
    for (size_t i = 0; i < fen_castling_rights.size(); i++) {
        char c = fen_castling_rights.at(i);
        switch (c) {
        case 'K':
            this->castling_rights += w_kingside;
            break;
        case 'Q':
            this->castling_rights += w_queenside;
            break;
        case 'k':
            this->castling_rights += b_kingside;
            break;
        case 'q':
            this->castling_rights += b_queenside;
            break;
        case '-':
            this->castling_rights = 0;
            break;
        default:
            std::cout << "An error occured while reading the fen -- illegal "
                         "castling rights string. char at fault: '"
                      << c << "'." << std::endl;
            break;
        }
    }

    std::string fen_ep_target_square = fen_split[3];

    if (fen_ep_target_square == "-") {
        this->ep_target_square = 0ULL;
    } else {
        int8_t index = 8 * char_to_rank(fen_ep_target_square.at(1)) + char_to_file(fen_ep_target_square.at(0));
        this->ep_target_square = 1ULL << index;
    }
    std::string fen_fifty_move_clock = fen_split[4];

    return;
}

void bitboard::pos_from_epd_line(std::string epd_l) {
    std::vector<std::string> epd_split;
    utility::split_string(&epd_split, epd_l);

    std::string fen_string;
    for (int i = 0; i < 4; i++) {
        fen_string += epd_split[i] + " ";
    }
    fen_string += "0 1";
    this->pos_from_fen(fen_string);
}

std::string bitboard::pos_to_fen() {
    std::string fen = "";
    // iterating over the ranks from top to bottom (8th to first)
    for (int rank = 7; rank >= 0; rank--) {
        // iterating over the squares in the rank...
        int gap = 0;
        for (int square = 0; square < 8; square++) {
            uint8_t piece = pieces[8 * rank + square];
            if (piece != EMPTY_PIECE) {
                if (gap > 0) {
                    fen += std::to_string(gap);
                }
                fen += piece_to_char(piece);
                gap = 0;
            } else {
                gap++;
            }
        }
        if (gap > 0) {
            fen += std::to_string(gap);
        }
        // print slashes between the ranks
        // don't print the last slash
        if (rank > 0) {
            fen += "/";
        }
    }
    fen += " ";
    // print out side to move
    fen += side_to_move ? "b" : "w";
    fen += " ";

    // print out castling rights
    if (castling_rights & w_kingside) {
        fen += "K";
    }
    if (castling_rights & w_queenside) {
        fen += "Q";
    }
    if (castling_rights & b_kingside) {
        fen += "k";
    }
    if (castling_rights & b_queenside) {
        fen += "q";
    }
    if (castling_rights == 0) {
        fen += "-";
    }

    fen += " ";

    // ep target
    if (ep_target_square != 0ULL) {
        int8_t index = BitScanForward64(ep_target_square);
        fen.append(bit_move::squares_to_string[index]);
    } else {
        fen += "-";
    }

    fen += " ";
    fen += std::to_string(fifty_move_rule_counter);
    fen += " ";
    fen += std::to_string(full_move_clock);

    return fen;
}

int bitboard::char_to_rank(char c) { return c - '0' - 1; }
int bitboard::char_to_file(char c) { return c - 'a'; }

int bitboard::piece_to_char(uint8_t piece) {
    switch (piece) {
    case EMPTY_PIECE:
        return ' ';
    case WHITE_KING:
        return 'K';
    case BLACK_KING:
        return 'k';
    case WHITE_PAWN:
        return 'P';
    case BLACK_PAWN:
        return 'p';
    case WHITE_KNIGHT:
        return 'N';
    case BLACK_KNIGHT:
        return 'n';
    case WHITE_BISHOP:
        return 'B';
    case BLACK_BISHOP:
        return 'b';
    case WHITE_ROOK:
        return 'R';
    case BLACK_ROOK:
        return 'r';
    case WHITE_QUEEN:
        return 'Q';
    case BLACK_QUEEN:
        return 'q';
    }
    return '?';
}

/**
 * Makes a so called "nullmove" on the board, meaning one side just passes the
 * turn to the other.
 *
 */
void bitboard::make_null_move() {
    bit_move nm = {};

    this->game_history.emplace_back(zobrist_key, pawn_hash_key, ep_target_square, nm, PST_score_MG, PST_score_EG,
                                    fifty_move_rule_counter, castling_rights);
    this->side_to_move = !this->side_to_move;
    this->zobrist_key ^= transposition_table::side_key;
    this->fifty_move_rule_counter = 0;
    this->ep_target_square = 0ULL;
}
/**
 * Undoes a nullmove.
 *
 */
void bitboard::unmake_null_move() {
    board_state bs = this->game_history.back();
    this->game_history.pop_back();
    // clang-format off
    this->side_to_move            = !this->side_to_move;
    this->fifty_move_rule_counter = bs.fifty_move_counter;
    this->zobrist_key             = bs.z_hash;
    this->pawn_hash_key           = bs.p_hash;
    this->PST_score_MG            = bs.PST_score_MG;
    this->PST_score_EG            = bs.PST_score_EG;
    this->ep_target_square        = bs.en_passant_target_square;
    // clang-format on
}

/**
 * @brief Checks if a square is attacked by the opponent.
 *
 * @param square the square to check
 * @param side_to_move the side to move
 * @return true if the square is attacked; false otherwise
 */
bool bitboard::is_square_attacked(int square, bool side_to_move) {
    if (attacks::pawn_attacks[!side_to_move][square] & bbs[PAWN][side_to_move]) {
        return true;
    }
    if (attacks::knight_attacks[square] & bbs[KNIGHT][side_to_move]) {
        return true;
    }
    if (attacks::get_bishop_attacks(square, occupancy[0] | occupancy[1]) &
        (bbs[BISHOP][side_to_move] | bbs[QUEEN][side_to_move])) {
        return true;
    }
    if (attacks::get_rook_attacks(square, occupancy[0] | occupancy[1]) &
        (bbs[ROOK][side_to_move] | bbs[QUEEN][side_to_move])) {
        return true;
    }
    if (attacks::king_attacks[square] & bbs[KING][side_to_move]) {
        return true;
    }
    return false;
}

// b2c3 a6e2 e1e2 a7b6 e2e3 a1a1 g2f3 e3e2

void bitboard::print_board() {
    unsigned long index = 0;
    std::string pieces_str[64] = {};
    if (bbs[KING][WHITE] != 0ULL) {
        index = BitScanForward64(bbs[KING][WHITE]);
        pieces_str[index] = "K";
    }
    if (bbs[KING][BLACK] != 0ULL) {
        index = BitScanForward64(this->bbs[KING][BLACK]);
        pieces_str[index] = "k";
    }
    uint64_t w_queens = this->bbs[QUEEN][WHITE];
    while (w_queens != 0ULL) {
        index = BitScanForward64(w_queens);
        pieces_str[index] = "Q";
        w_queens &= w_queens - 1;
    }
    uint64_t b_queens = this->bbs[QUEEN][BLACK];
    while (b_queens != 0ULL) {
        index = BitScanForward64(b_queens);
        pieces_str[index] = "q";
        b_queens &= b_queens - 1;
    }
    uint64_t w_rooks = this->bbs[ROOK][WHITE];
    while (w_rooks != 0ULL) {
        index =  BitScanForward64(w_rooks);
        pieces_str[index] = "R";
        w_rooks &= w_rooks - 1;
    }
    uint64_t b_rooks = this->bbs[ROOK][BLACK];
    while (b_rooks != 0ULL) {
        index = BitScanForward64(b_rooks);
        pieces_str[index] = "r";
        b_rooks &= b_rooks - 1;
    }
    uint64_t w_bishops = this->bbs[BISHOP][WHITE];
    while (w_bishops != 0ULL) {
        index = BitScanForward64(w_bishops);
        pieces_str[index] = "B";
        w_bishops &= w_bishops - 1;
    }
    uint64_t b_bishops = this->bbs[BISHOP][BLACK];
    while (b_bishops != 0ULL) {
        index = BitScanForward64(b_bishops);
        pieces_str[index] = "b";
        b_bishops &= b_bishops - 1;
    }
    uint64_t w_knights = this->bbs[KNIGHT][WHITE];
    while (w_knights != 0ULL) {
        index = BitScanForward64(w_knights);
        pieces_str[index] = "N";
        w_knights &= w_knights - 1;
    }
    uint64_t b_knights = this->bbs[KNIGHT][BLACK];
    while (b_knights != 0ULL) {
        index = BitScanForward64(b_knights);
        pieces_str[index] = "n";
        b_knights &= b_knights - 1;
    }
    uint64_t w_pawns = this->bbs[PAWN][WHITE];
    while (w_pawns != 0ULL) {
        index = BitScanForward64(w_pawns);
        pieces_str[index] = "P";
        w_pawns &= w_pawns - 1;
    }
    uint64_t b_pawns = this->bbs[PAWN][BLACK];
    while (b_pawns != 0ULL) {
        index = BitScanForward64(b_pawns);
        pieces_str[index] = "p";
        b_pawns &= b_pawns - 1;
    }
    std::cout << std::endl;
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            if (pieces_str[i * 8 + j] != "") {
                std::cout << pieces_str[i * 8 + j] << " ";
            } else {
                std::cout << "  ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    for (board_state s : game_history) {
        std::cout << bit_move::to_string(s.last_move) << " ";
    }
    std::cout << std::endl;
    std::cout << "FEN: " << pos_to_fen() << std::endl;
    std::cout << "Hash: " << zobrist_key << std::endl;
    std::cout << std::endl;
}

/**
 * Makes a given move on the board. (Updates zobrist keys accordingly)
 *
 * This definition follows the definition seen in Koivisto 9.0.
 *
 * @param m pointer to the move
 */
void bitboard::make_move(bit_move *m) {
    // save the current game state in the game history vector
    game_history.emplace_back(zobrist_key,             // hash table key
                              pawn_hash_key,           // pawn hash table key
                              ep_target_square,        // the en-passant target square
                              *m,                      // last move
                              PST_score_MG,            // incremental update PST midgame score
                              PST_score_EG,            // incremental update PST endgame score
                              fifty_move_rule_counter, // 50 move counter for restoration
                              castling_rights);        // castling rights char
    // extract move details
    // clang-format off
    const uint8_t origin        = m->get_origin();
    const uint8_t target        = m->get_target();
    const uint8_t flags         = m->get_flags();
    const uint8_t piece_type    = m->get_piece_type();
    const uint8_t captured_type = m->get_captured_type();
    // clang-format on
    const bool is_capture = captured_type != EMPTY;
    // convert origin & target to bitmasks
    const int8_t factor = (side_to_move) ? -1 : 1;
    const uint8_t piece = pieces[origin];

    this->fifty_move_rule_counter++;
    this->full_move_clock += side_to_move;

    this->ep_target_square = 0ULL;
    if (is_capture) { // if is capture
        // reset 50-move rule counter
        fifty_move_rule_counter = 0;
        // if unmoved rook is captured => reset respective castling rights
        if (captured_type == ROOK) {
            if (side_to_move) {
                if (target == A1) {
                    castling_rights &= ~w_queenside;
                } else if (target == H1) {
                    castling_rights &= ~w_kingside;
                }
            } else {
                if (target == A8) {
                    castling_rights &= ~b_queenside;
                } else if (target == H8) {
                    castling_rights &= ~b_kingside;
                }
            }
        }
    }

    // updating the zobrist key for the switch of the moving side
    zobrist_key ^= transposition_table::side_key;

    if (piece_type == PAWN) {
        fifty_move_rule_counter = 0;

        if (flags == bit_move::double_pawn_push) {
            // update ep target square
            ep_target_square = 1ULL << (origin + 8 * factor);
            // std::cout << ep_target_square << std::endl;
        }
        // if move is a promotion
        if (flags >= bit_move::knight_promotion) {
            side_to_move = 1 - side_to_move;
            unset_piece<true, true>(origin);
            if (flags >= bit_move::knight_capture_promotion) {
                replace_piece<true, true>(flags - 11 + (!side_to_move) * BLACK_PAWN, target);
            } else {
                place_piece<true, true>(flags - 7 + (!side_to_move) * BLACK_PAWN, target);
            }

            return;
        } else if (flags == bit_move::ep_capture) {
            side_to_move = 1 - side_to_move;
            unset_piece<true, true>(target - 8 * factor);
            unset_piece<true, true>(origin);
            place_piece<true, true>(piece, target);
            return;
        }
    } else if (piece_type == KING) {
        // if king moves the side to moves loses the right to castle
        castling_rights &= ~(1ULL << (side_to_move * 2));
        castling_rights &= ~(1ULL << (side_to_move * 2 + 1));

        if (flags == bit_move::kingside_castle || flags == bit_move::queenside_castle) {
            uint8_t rook_origin = origin + ((flags == bit_move::queenside_castle) ? -4 : 3);
            uint8_t rook_target = target + ((flags == bit_move::queenside_castle) ? 1 : -1);
            unset_piece<true, true>(rook_origin);
            place_piece<true, true>(ROOK + (side_to_move)*BLACK_PAWN, rook_target);
        }
        king_positions[side_to_move] = target;
        side_to_move = !side_to_move;

        unset_piece<true, true>(origin);
        if (is_capture) {
            unset_piece<true, true>(target);
        }
        place_piece<true, true>(piece, target);

        return;
    }

    // If move is rook move, update castling rights
    else if (piece_type == ROOK) {
        if (origin == A1) {
            castling_rights &= ~(w_queenside);
        } else if (origin == H1) {
            castling_rights &= ~(w_kingside);
        } else if (origin == A8) {
            castling_rights &= ~(b_queenside);
        } else if (origin == H8) {
            castling_rights &= ~(b_kingside);
        }
    }

    side_to_move = !side_to_move;
    unset_piece<true, true>(origin);

    if (is_capture) {
        unset_piece<true, true>(target);
        place_piece<true, true>(piece, target);
    } else {
        place_piece<true, true>(piece, target);
    }
}

void bitboard::unmake_move() {
    // pop previous board state from game history vector

    board_state prev_board_state = game_history.back();
    game_history.pop_back();

    // restore previous board state
    // clang-format off
    side_to_move            = !side_to_move;
    full_move_clock        -= side_to_move;
    fifty_move_rule_counter = prev_board_state.fifty_move_counter;
    ep_target_square        = prev_board_state.en_passant_target_square;
    zobrist_key             = prev_board_state.z_hash;
    pawn_hash_key           = prev_board_state.p_hash;
    castling_rights         = prev_board_state.castling_rights;
    PST_score_MG            = prev_board_state.PST_score_MG;
    PST_score_EG            = prev_board_state.PST_score_EG;
    // clang-format on

    bit_move prev_move = prev_board_state.last_move;
    // declare useful const variables
    const uint8_t origin = prev_move.get_origin();
    const uint8_t target = prev_move.get_target();
    const uint8_t piece_type = prev_move.get_piece_type();
    const uint8_t captured_type = prev_move.get_captured_type();
    // flags indicating move type (castling, ep, promotion, captures ...)
    const uint8_t flags = prev_move.get_flags();
    const uint8_t factor = (side_to_move == WHITE) ? 1 : -1;
    const uint8_t piece = piece_type + (side_to_move)*BLACK_PAWN;

    const bool is_capture = captured_type != EMPTY;
    const bool is_ep = flags == bit_move::ep_capture;

    if (piece_type == KING) {
        king_positions[side_to_move] = origin;
    }

    if (is_ep) {
        place_piece<false, false>((!side_to_move) * BLACK_PAWN, target - 8 * factor);
    }

    if (flags == bit_move::queenside_castle || flags == bit_move::kingside_castle) {
        const uint8_t rook_origin = origin + (flags == bit_move::queenside_castle ? -4 : 3);
        const uint8_t rook_target = target + (flags == bit_move::queenside_castle ? 1 : -1);

        place_piece<false, false>(ROOK + BLACK_PAWN * side_to_move, rook_origin);
        unset_piece<false, false>(rook_target);
    }

    if (is_capture && (!is_ep)) {
        unset_piece<false, false>(target);
        place_piece<false, false>(captured_type + (!side_to_move) * BLACK_PAWN, target);
    } else {
        unset_piece<false, false>(target);
    }

    place_piece<false, false>(piece, origin);
}

template <bool update_zobrist, bool update_score> void bitboard::place_piece(uint8_t piece, uint8_t target) {
    //
    const uint8_t type = piece % BLACK_PAWN;
    const bool color = piece >= BLACK_PAWN;
    const uint64_t target_bb = (1ULL) << target;

    pieces[target] = piece;
    types[target] = type;

    // set piece bb
    bbs[type][color] |= target_bb;
    // set piece to color occupancy bb
    occupancy[color] |= target_bb;
    // set piece to occupancy bb
    occupancy[2] |= target_bb;

    // update the zobrist hash key => important
    if (update_zobrist) {
        zobrist_key ^= transposition_table::piece_keys[piece][target];
        if (type == PAWN || type == KING) {
            pawn_hash_key ^= transposition_table::piece_keys[piece][target];
        }
    }
    if (update_score) {
        this->PST_score_MG +=
            weights::piece_values[MIDGAME][piece] + weights::piece_square_tables[MIDGAME][piece][target];
        this->PST_score_EG +=
            weights::piece_values[ENDGAME][piece] + weights::piece_square_tables[ENDGAME][piece][target];
    }
}

template <bool update_zobrist, bool update_score> void bitboard::unset_piece(uint8_t target) {
    const uint8_t piece = pieces[target];
    const uint8_t type = piece % BLACK_PAWN;
    const bool color = piece >= BLACK_PAWN;

    const uint64_t target_bb = ~((1ULL) << target);

    pieces[target] = EMPTY_PIECE;
    types[target] = EMPTY;

    // set piece bb
    bbs[type][color] &= target_bb;
    // set color occupancy
    occupancy[color] &= target_bb;
    // set union occupancy
    occupancy[2] &= target_bb;

    // update the zobrist hash key
    if (update_zobrist) {
        zobrist_key ^= transposition_table::piece_keys[piece][target];
        if (type == PAWN || type == KING) {
            pawn_hash_key ^= transposition_table::piece_keys[piece][target];
        }
    }
    if (update_score) {
        this->PST_score_MG -=
            weights::piece_values[MIDGAME][piece] + weights::piece_square_tables[MIDGAME][piece][target];
        this->PST_score_EG -=
            weights::piece_values[ENDGAME][piece] + weights::piece_square_tables[ENDGAME][piece][target];
    }
}

/**
 * @brief replaces a piece on the board.
 *
 * @param piece piece to place
 * @param target target square (any piece is removed from this square first)
 */
template <bool update_zobrist, bool update_score> void bitboard::replace_piece(uint8_t piece, uint8_t target) {
    unset_piece<update_zobrist, update_score>(target);
    place_piece<update_zobrist, update_score>(piece, target);
}

uint8_t bitboard::piece_type_from_index(unsigned long i) { return types[i]; }
