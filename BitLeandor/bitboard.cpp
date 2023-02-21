#include "bitboard.h"

bitboard::bitboard()
{
	this->game_history.reserve(512);
}

bitboard::~bitboard()
{
}

bool bitboard::is_sane()
{
	if (bbs[KING][WHITE] == 0ULL || bbs[KING][BLACK] == 0ULL) {
		print_board();
	}
	uint64_t hash_key = 0ULL;
	for (int i = 0; i < 64; i++) {
		uint8_t piece = pieces[i];
		uint8_t type = types[i];
		if (type != EMPTY) {
			if ((bbs[type][WHITE] & (1ULL << i)) == 0 && (bbs[type][BLACK] & (1ULL << i)) == 0) {
				std::cout << "Type " << ((int)type) << " not in bitboard" << std::endl;
				std::cout << i << std::endl;
				print_board();
				return false;
			}
		}
		if (piece != EMPTY_PIECE) {
			hash_key ^= transposition_table::piece_keys[piece][i];
			if ((bbs[piece % BLACK_PAWN][piece >= BLACK_PAWN] & (1ULL << i)) == 0) {
				std::cout << "Piece " << ((int)piece) << " not in bitboard" << std::endl;
				std::cout << i << std::endl;
				print_board();
				return false;
			}
		}
		else {
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
		// std::cout << "hash key mismatch: correct ->" << hash_key << " incorrect -> " << zobrist_key << std::endl;
		// print_board();
		// return false;
	}
	return true;
}

void bitboard::pos_from_fen(std::string fen) {
	std::vector<std::string>fen_split;
	utility::split_string(&fen_split, fen);
	this->zobrist_key = 0ULL;
	for (int i = 0; i < 6; i++) {
		this->bbs[i][0] = 0ULL;
		this->bbs[i][1] = 0ULL;
	}

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
			}
			else {
				switch (c)
				{
				case 'P':
					this->bbs[PAWN][0] |= set_bit(board_index);
					this->types[board_index] = PAWN;
					this->pieces[board_index] = WHITE_PAWN;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_PAWN][board_index];
					this->pawn_hash_key ^= transposition_table::piece_keys[WHITE_PAWN][board_index];
					break;
				case 'p':
					this->bbs[PAWN][1] |= set_bit(board_index);
					this->types[board_index] = PAWN;
					this->pieces[board_index] = BLACK_PAWN;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_PAWN][board_index];
					this->pawn_hash_key ^= transposition_table::piece_keys[BLACK_PAWN][board_index];
					break;
				case 'N':
					this->bbs[KNIGHT][0] |= set_bit(board_index);
					this->types[board_index] = KNIGHT;
					this->pieces[board_index] = WHITE_KNIGHT;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_KNIGHT][board_index];
					break;
				case 'n':
					this->bbs[KNIGHT][1] |= set_bit(board_index);
					this->types[board_index] = KNIGHT;
					this->pieces[board_index] = BLACK_KNIGHT;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_KNIGHT][board_index];
					break;
				case 'B':
					this->bbs[BISHOP][0] |= set_bit(board_index);
					this->types[board_index] = BISHOP;
					this->pieces[board_index] = WHITE_BISHOP;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_BISHOP][board_index];
					break;
				case 'b':
					this->bbs[BISHOP][1] |= set_bit(board_index);
					this->types[board_index] = BISHOP;
					this->pieces[board_index] = BLACK_BISHOP;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_BISHOP][board_index];
					break;
				case 'R':
					this->bbs[ROOK][0] |= set_bit(board_index);
					this->types[board_index] = ROOK;
					this->pieces[board_index] = WHITE_ROOK;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_ROOK][board_index];
					break;
				case 'r':
					this->bbs[ROOK][1] |= set_bit(board_index);
					this->types[board_index] = ROOK;
					this->pieces[board_index] = BLACK_ROOK;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_ROOK][board_index];
					break;
				case 'Q':
					this->bbs[QUEEN][0] |= set_bit(board_index);
					this->types[board_index] = QUEEN;
					this->pieces[board_index] = WHITE_QUEEN;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_QUEEN][board_index];
					break;
				case 'q':
					this->bbs[QUEEN][1] |= set_bit(board_index);
					this->types[board_index] = QUEEN;
					this->pieces[board_index] = BLACK_QUEEN;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_QUEEN][board_index];
					break;
				case 'K':
					this->bbs[KING][0] |= set_bit(board_index);
					this->types[board_index] = KING;
					this->pieces[board_index] = WHITE_KING;
					this->king_positions[WHITE] = board_index;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_KING][board_index];
					this->pawn_hash_key ^= transposition_table::piece_keys[WHITE_KING][board_index];
					break;
				case 'k':
					this->bbs[KING][1] |= set_bit(board_index);
					this->types[board_index] = KING;
					this->pieces[board_index] = BLACK_KING;
					this->king_positions[BLACK] = board_index;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_KING][board_index];
					this->pawn_hash_key ^= transposition_table::piece_keys[BLACK_KING][board_index];
					break;
				default:
					std::cout << "An error occured while reading the fen -- illegal position string. char at fault: '" << c << "'." << std::endl;
					break;
				}
			}
			fen_pos_index++;
		}
		fen_pos_index++;
	}

	this->occupancy[0] = this->bbs[PAWN][0] | this->bbs[KNIGHT][0] | this->bbs[BISHOP][0] | this->bbs[ROOK][0] | this->bbs[QUEEN][0] | this->bbs[KING][0];
	this->occupancy[1] = this->bbs[PAWN][1] | this->bbs[KNIGHT][1] | this->bbs[BISHOP][1] | this->bbs[ROOK][1] | this->bbs[QUEEN][1] | this->bbs[KING][1];
	this->occupancy[2] = this->occupancy[0] | this->occupancy[1];

	std::string fen_side_to_move = fen_split[1];
	this->side_to_move = fen_side_to_move == "b";
	if (side_to_move) {
		this->zobrist_key ^= transposition_table::side_key;
	}

	std::string fen_castling_rights = fen_split[2];
	for (size_t i = 0; i < fen_castling_rights.size(); i++) {
		char c = fen_castling_rights.at(i);
		switch (c)
		{
		case 'K':
			this->castling_rights += w_kingside;
			// this->zobrist_key ^= transposition_table::castling_keys[0];
			break;
		case 'Q':
			this->castling_rights += w_queenside;
			// this->zobrist_key ^= transposition_table::castling_keys[1];
			break;
		case 'k':
			this->castling_rights += b_kingside;
			// this->zobrist_key ^= transposition_table::castling_keys[2];
			break;
		case 'q':
			this->castling_rights += b_queenside;
			// this->zobrist_key ^= transposition_table::castling_keys[3];
			break;
		case '-':
			this->castling_rights = 0;
			break;
		default:
			std::cout << "An error occured while reading the fen -- illegal castling rights string. char at fault: '" << c << "'." << std::endl;
			break;
		}
	}

	//std::cout << (int) castling_rights << std::endl;

	std::string fen_ep_target_square = fen_split[3];
	if (fen_ep_target_square == "-") {
		this->ep_target_square = 0ULL;
	}
	else {
		int8_t index = 8 * char_to_rank(fen_ep_target_square.at(1)) + char_to_file(fen_ep_target_square.at(0));
		this->ep_target_square = 1ULL << index;
		// this->zobrist_key ^= transposition_table::en_passant_keys[this->ep_target_square % 8];
	}
	// std::cout << "ep_target_square: " << this->ep_target_square << std::endl;
	std::string fen_fifty_move_clock = fen_split[4];

	return;
}

std::string bitboard::pos_to_fen()
{
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
			}
			else {
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
	}
	else {
		fen += "-";
	}

	fen += " ";
	fen += std::to_string(fifty_move_rule_counter);
	fen += " ";
	fen += std::to_string(full_move_clock);

	return fen;
}

int bitboard::char_to_rank(char c) {
	return c - '0' - 1;
}
int bitboard::char_to_file(char c) {
	return c - 'a';
}

int bitboard::piece_to_char(uint8_t piece)
{
	switch (piece)
	{
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
 * Makes a so called "nullmove" on the board, meaning one side just passes the turn to the other.
 * 
 */
void bitboard::make_null_move() {
	bit_move nm = {};

	this->game_history.emplace_back(zobrist_key, ep_target_square, nm, fifty_move_rule_counter, castling_rights);
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
	this->side_to_move = !this->side_to_move;
	this->fifty_move_rule_counter = bs.fifty_move_counter;
	this->zobrist_key = bs.z_hash;
	this->ep_target_square = bs.en_passant_target_square;
}



/**
 * \brief Checks if a square is attacked by the opponent.
 * 
 * \param square the square to check
 * \param side_to_move the side to move
 * \return true if the square is attacked; false otherwise
 */
bool bitboard::is_square_attacked(int square, bool side_to_move) {
	if (attacks::pawn_attacks[!side_to_move][square] & bbs[PAWN][side_to_move]) {
		return 1;
	}
	if (attacks::knight_attacks[square] & bbs[KNIGHT][side_to_move]) {
		return 1;
	}
	if (attacks::get_bishop_attacks(square, occupancy[0] | occupancy[1]) & (bbs[BISHOP][side_to_move] | bbs[QUEEN][side_to_move])) {
		return 1;
	}
	if (attacks::get_rook_attacks(square, occupancy[0] | occupancy[1]) & (bbs[ROOK][side_to_move] | bbs[QUEEN][side_to_move])) {
		return 1;
	}
	if (attacks::king_attacks[square] & bbs[KING][side_to_move]) {
		return 1;
	}
	return 0;
}

// b2c3 a6e2 e1e2 a7b6 e2e3 a1a1 g2f3 e3e2

void bitboard::print_board()
{
	unsigned long index = 0;
	std::string pieces_str[64] = { };
	if (bbs[KING][WHITE] != 0ULL) {
		_BitScanForward64(&index, bbs[KING][WHITE]);
		pieces_str[index] = "K";
	}
	if (bbs[KING][BLACK] != 0ULL) {
		_BitScanForward64(&index, this->bbs[KING][BLACK]);
		pieces_str[index] = "k";
	}
	uint64_t w_queens = this->bbs[QUEEN][WHITE];
	while (w_queens != 0ULL) {
		_BitScanForward64(&index, w_queens);
		pieces_str[index] = "Q";
		w_queens &= w_queens - 1;
	}
	uint64_t b_queens = this->bbs[QUEEN][BLACK];
	while (b_queens != 0ULL) {
		_BitScanForward64(&index, b_queens);
		pieces_str[index] = "q";
		b_queens &= b_queens - 1;
	}
	uint64_t w_rooks = this->bbs[ROOK][WHITE];
	while (w_rooks != 0ULL) {
		_BitScanForward64(&index, w_rooks);
		pieces_str[index] = "R";
		w_rooks &= w_rooks - 1;
	}
	uint64_t b_rooks = this->bbs[ROOK][BLACK];
	while (b_rooks != 0ULL) {
		_BitScanForward64(&index, b_rooks);
		pieces_str[index] = "r";
		b_rooks &= b_rooks - 1;
	}
	uint64_t w_bishops = this->bbs[BISHOP][WHITE];
	while (w_bishops != 0ULL) {
		_BitScanForward64(&index, w_bishops);
		pieces_str[index] = "B";
		w_bishops &= w_bishops - 1;
	}
	uint64_t b_bishops = this->bbs[BISHOP][BLACK];
	while (b_bishops != 0ULL) {
		_BitScanForward64(&index, b_bishops);
		pieces_str[index] = "b";
		b_bishops &= b_bishops - 1;
	}
	uint64_t w_knights = this->bbs[KNIGHT][WHITE];
	while (w_knights != 0ULL) {
		_BitScanForward64(&index, w_knights);
		pieces_str[index] = "N";
		w_knights &= w_knights - 1;
	}
	uint64_t b_knights = this->bbs[KNIGHT][BLACK];
	while (b_knights != 0ULL) {
		_BitScanForward64(&index, b_knights);
		pieces_str[index] = "n";
		b_knights &= b_knights - 1;
	}
	uint64_t w_pawns = this->bbs[PAWN][WHITE];
	while (w_pawns != 0ULL) {
		_BitScanForward64(&index, w_pawns);
		pieces_str[index] = "P";
		w_pawns &= w_pawns - 1;
	}
	uint64_t b_pawns = this->bbs[PAWN][BLACK];
	while (b_pawns != 0ULL) {
		_BitScanForward64(&index, b_pawns);
		pieces_str[index] = "p";
		b_pawns &= b_pawns - 1;
	}
	std::cout << std::endl;
	for (int i = 7; i >= 0; i--) {
		for (int j = 0; j < 8; j++) {
			if (pieces_str[i * 8 + j] != "") {
				std::cout << pieces_str[i * 8 + j] << " ";
			}
			else {
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
void bitboard::make_move(bit_move* m)
{
	// save the current game state in the game history vector
	this->game_history.emplace_back(this->zobrist_key, this->ep_target_square, *m, this->fifty_move_rule_counter, this->castling_rights);
	// extract move details
	const uint8_t origin = m->get_origin();
	const uint8_t target = m->get_target();
	const uint8_t flags = m->get_flags();
	const uint8_t piece_type = m->get_piece_type();
	const uint8_t captured_type = m->get_captured_type();
	// convert origin & target to bitmasks
	const uint64_t origin_bit = set_bit(origin);
	const uint64_t target_bit = set_bit(target);
	const int8_t factor = (side_to_move) ? -1 : 1;
	const uint8_t piece = pieces[origin];

	this->fifty_move_rule_counter++;
	this->full_move_clock += side_to_move;

	this->ep_target_square = 0ULL;
	if (captured_type != EMPTY) { // if is capture
		// reset 50-move rule counter 
		fifty_move_rule_counter = 0;
		// if unmoved rook is captured => reset respective castling rights
		if (captured_type == ROOK) {
			if (side_to_move) {
				if (target == A1) {
					castling_rights &= ~w_queenside;
				}
				else if (target == H1) {
					castling_rights &= ~w_kingside;
				}
			}
			else {
				if (target == A8) {
					castling_rights &= ~b_queenside;
				}
				else if (target == H8) {
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
			unset_piece<true>(origin);
			if (flags >= bit_move::knight_capture_promotion) {
				replace_piece<true>(flags - 11 + (!side_to_move) * BLACK_PAWN, target);
			}
			else {
				place_piece<true>(flags - 7 + (!side_to_move) * BLACK_PAWN, target);
			}

			return;
		}
		else if (flags == bit_move::ep_capture) {
			side_to_move = 1 - side_to_move;
			unset_piece<true>(target - 8 * factor);
			unset_piece<true>(origin);
			place_piece<true>(piece, target);
			return;
		}
	}
	else if (piece_type == KING) {
		// if king moves the side to moves loses the right to castle
		castling_rights &= ~(1ULL << (side_to_move * 2));
		castling_rights &= ~(1ULL << (side_to_move * 2 + 1));

		if (flags == bit_move::kingside_castle || flags == bit_move::queenside_castle) {
			uint8_t rook_origin = origin + ((flags == bit_move::queenside_castle) ? -4 : 3);
			uint8_t rook_target = target + ((flags == bit_move::queenside_castle) ? 1 : -1);
			unset_piece<true>(rook_origin);
			place_piece<true>(ROOK + (side_to_move)*BLACK_PAWN, rook_target);
		}
		king_positions[side_to_move] = target;
		side_to_move = !side_to_move;

		unset_piece<true>(origin);
		if (flags >= bit_move::capture) {
			unset_piece<true>(target);
		}
		place_piece<true>(piece, target);
		
		return;
	}



	else if (piece_type == ROOK) {
		if (origin == A1) {
			castling_rights &= ~(w_queenside);
		}
		else if (origin == H1) {
			castling_rights &= ~(w_kingside);
		}
		else if (origin == A8) {
			castling_rights &= ~(b_queenside);
		}
		else if (origin == H8) {
			castling_rights &= ~(b_kingside);
		}
	}

	side_to_move = !side_to_move;
	unset_piece<true>(origin);

	if (flags >= bit_move::capture && flags != bit_move::ep_capture) {
		unset_piece<true>(target);
		place_piece<true>(piece, target);
	}
	else {
		place_piece<true>(piece, target);
	}

	/*if (!is_sane()) {
		std::cout << "not sane " << bit_move::to_string(*m) << std::endl;
	}*/
	// position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5c6 d7c6 d2d4 e5d4 d1d4 d8d4 f3d4 g8f6 b1c3 f8b4 f2f3 b4c3 b2c3 e8g8 c1f4 c6c5 d4b3 f8e8 b3c5 f6d5 f4g3 d5c3 g3c7 c3d5 c7d6 b7b6 c5a4 b6b5 e1c1 d5e3 a4b6 e3d1 b6a8 d1e3 h1g1 c8e6 a8c7 e8c8 a2a3 g7g5 g2g4 e6d7 e4e5 d7c6 c7a6 c6f3 a6c7 f3c6 g1g3 e3d5 c7d5 c6d5 c1d2 d5e6 d6e7 h7h6 h2h3 e6c4 e7f6 c8c7 d2c3 c4e6 c3b2 e6c4 g3g1 c7d7 b2c3 d7a7 g1a1 a7a4 c3b2 g8h7 a1d1 c4e6 d1d8 a4f4 d8h8 h7g6 h8g8 g6h7 g8g7 h7h8 g7f7 h8g8 f7g7 g8f8 g7g6 b5b4 g6h6 b4a3 b2a3 f4f3 a3b4 f3e3 h6h5 f8e8 h5g5 e3h3 g5g7 e8f8 g4g5 h3h2 g5g6 h2c2 g7e7 c2b2 b4c5 b2c2 c5d6 e6a2 g6g7 f8g8

}

void bitboard::unmake_move()
{
	// pop previous board state from game history vector

	board_state prev_board_state = game_history.back();
	game_history.pop_back();

	// restore previous board state
	this->side_to_move = !side_to_move;
	this->full_move_clock -= side_to_move;
	this->fifty_move_rule_counter = prev_board_state.fifty_move_counter;
	this->ep_target_square = prev_board_state.en_passant_target_square;
	this->zobrist_key = prev_board_state.z_hash;
	this->castling_rights = prev_board_state.castling_rights;

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

	if (piece_type == KING) {
		king_positions[side_to_move] = origin;
	}

	if (flags == bit_move::ep_capture) {
		place_piece<false>((!side_to_move) * BLACK_PAWN, target - 8 * factor);
	}

	if (flags == bit_move::queenside_castle || flags == bit_move::kingside_castle) {
		const uint8_t rook_origin = origin + (flags == bit_move::queenside_castle ? -4 : 3);
		const uint8_t rook_target = target + (flags == bit_move::queenside_castle ? 1 : -1);

		place_piece<false>(ROOK + BLACK_PAWN * side_to_move, rook_origin);
		unset_piece<false>(rook_target);
	}

	if (captured_type != EMPTY && flags != bit_move::ep_capture) {
		unset_piece<false>(target);
		place_piece<false>(captured_type + (!side_to_move) * BLACK_PAWN, target);
	}
	else {
		unset_piece<false>(target);
	}

	place_piece<false>(piece, origin);

	/*if (!is_sane()) {
		std::cout << "not sane " << bit_move::to_string(prev_move) << std::endl;
	}*/

}

/**
 * Places a piece on the board, updating the zobrist key accordingly.
 *
 * \param piece piece to be placed
 * \param target position where piece should be placed
 */
template<bool update_zobrist>
void bitboard::place_piece(uint8_t piece, uint8_t target)
{
	pieces[target] = piece;

	const uint8_t type = piece % BLACK_PAWN;
	types[target] = type;

	const bool color = (piece >= BLACK_PAWN);

	const uint64_t target_bb = (1ULL) << target;


	// set piece bb
	bbs[type][color] |= target_bb;
	// set piece to color occupancy bb
	occupancy[color] |= target_bb;
	// set piece to occupancy bb
	occupancy[2] |= target_bb;

	// update the zobrist hash key => important
	if (update_zobrist) {
		zobrist_key ^= transposition_table::piece_keys[piece][target];
	}
}

template<bool update_zobrist>
void bitboard::unset_piece(uint8_t target)
{
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
	}
}

/**
 * replaces a piece on the board.
 *
 * \param piece piece to place
 * \param target target square (any piece is removed from this square first)
 */
template<bool update_zobrist>
void bitboard::replace_piece(uint8_t piece, uint8_t target)
{
	unset_piece<update_zobrist>(target);
	place_piece<update_zobrist>(piece, target);
}

uint8_t bitboard::piece_type_from_index(unsigned long i)
{
	return types[i];
}

