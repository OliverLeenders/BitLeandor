#include "bitboard.h"

bitboard::bitboard()
{
	this->game_history.reserve(512);
}

bitboard::~bitboard()
{
}

void bitboard::pos_from_fen(std::string fen) {
	std::vector<std::string>fen_split;
	Utility::split_string(&fen_split, fen);
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
					this->bbs[PAWN][0] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = PAWN;
					this->pieces[board_index] = WHITE_PAWN;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_PAWN][board_index];
					break;
				case 'p':
					this->bbs[PAWN][1] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = PAWN;
					this->pieces[board_index] = BLACK_PAWN;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_PAWN][board_index];
					break;
				case 'N':
					this->bbs[KNIGHT][0] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = KNIGHT;
					this->pieces[board_index] = WHITE_KNIGHT;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_KNIGHT][board_index];
					break;
				case 'n':
					this->bbs[KNIGHT][1] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = KNIGHT;
					this->pieces[board_index] = BLACK_KNIGHT;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_KNIGHT][board_index];
					break;
				case 'B':
					this->bbs[BISHOP][0] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = BISHOP;
					this->pieces[board_index] = WHITE_BISHOP;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_BISHOP][board_index];
					break;
				case 'b':
					this->bbs[BISHOP][1] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = BISHOP;
					this->pieces[board_index] = BLACK_BISHOP;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_BISHOP][board_index];
					break;
				case 'R':
					this->bbs[ROOK][0] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = ROOK;
					this->pieces[board_index] = WHITE_ROOK;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_ROOK][board_index];
					break;
				case 'r':
					this->bbs[ROOK][1] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = ROOK;
					this->pieces[board_index] = BLACK_ROOK;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_ROOK][board_index];
					break;
				case 'Q':
					this->bbs[QUEEN][0] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = QUEEN;
					this->pieces[board_index] = WHITE_QUEEN;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_QUEEN][board_index];
					break;
				case 'q':
					this->bbs[QUEEN][1] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = QUEEN;
					this->pieces[board_index] = BLACK_QUEEN;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_QUEEN][board_index];
					break;
				case 'K':
					this->bbs[KING][0] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = KING;
					this->pieces[board_index] = WHITE_KING;
					this->zobrist_key ^= transposition_table::piece_keys[WHITE_KING][board_index];
					break;
				case 'k':
					this->bbs[KING][1] |= bitboard_util::set_bit(board_index);
					this->types[board_index] = KING;
					this->pieces[board_index] = BLACK_KING;
					this->zobrist_key ^= transposition_table::piece_keys[BLACK_KING][board_index];
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
	for (int i = 0; i < fen_castling_rights.size(); i++) {
		char c = fen_castling_rights.at(i);
		switch (c)
		{
		case 'K':
			this->castling_rights += w_kingside;
			this->zobrist_key ^= transposition_table::castling_keys[0];
			break;
		case 'Q':
			this->castling_rights += w_queenside;
			this->zobrist_key ^= transposition_table::castling_keys[1];
			break;
		case 'k':
			this->castling_rights += b_kingside;
			this->zobrist_key ^= transposition_table::castling_keys[2];
			break;
		case 'q':
			this->castling_rights += b_queenside;
			this->zobrist_key ^= transposition_table::castling_keys[3];
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
		this->ep_target_square = -1;
	}
	else {
		this->ep_target_square = 8 * char_to_rank(fen_ep_target_square.at(1)) + char_to_file(fen_ep_target_square.at(0));
		this->zobrist_key ^= transposition_table::en_passant_keys[this->ep_target_square % 8];
	}
	// std::cout << "ep_target_square: " << this->ep_target_square << std::endl;
	std::string fen_fifty_move_clock = fen_split[4];

	return;
}

int bitboard::char_to_rank(char c) {
	return c - '0' - 1;
}
int bitboard::char_to_file(char c) {
	return c - 'a';
}

void bitboard::make_null_move() {
	this->side_to_move = !this->side_to_move;
	this->zobrist_key ^= transposition_table::side_key;
}

void bitboard::unmake_null_move() {
	this->side_to_move = !this->side_to_move;
	this->zobrist_key ^= transposition_table::side_key;
}

void bitboard::update_zobrist_key(bit_move* m)
{
	uint8_t origin = m->get_origin();
	uint8_t target = m->get_target();
	uint8_t flags = m->get_flags();
	uint8_t piece = (side_to_move) ? BLACK_KNIGHT + m->get_piece_type() : m->get_piece_type();
	this->zobrist_key ^= transposition_table::piece_keys[piece][origin];
	if (flags < bit_move::knight_promotion) {
		this->zobrist_key ^= transposition_table::piece_keys[piece][target];
	}
	else {
		uint8_t promotion_piece = piece + flags + 1;
		this->zobrist_key ^= transposition_table::piece_keys[promotion_piece][target];
	}
	if (flags == bit_move::double_pawn_push) {
		this->zobrist_key ^= transposition_table::en_passant_keys[target % 8];
	}
	if (flags == bit_move::kingside_castle) {
		this->zobrist_key ^= transposition_table::piece_keys[piece - 2][target - 1];
		this->zobrist_key ^= transposition_table::piece_keys[piece - 2][target + 1];
	}
	if (flags == bit_move::queenside_castle) {
		this->zobrist_key ^= transposition_table::piece_keys[piece - 2][target + 1];
		this->zobrist_key ^= transposition_table::piece_keys[piece - 2][target - 2];
	}
	if (flags == bit_move::ep_capture) {
		uint8_t ep_target = target + ((this->side_to_move) ? -8 : 8);
		this->zobrist_key ^= transposition_table::piece_keys[(side_to_move) ? BLACK_PAWN : WHITE_PAWN][ep_target];
	}
	else if (flags % 8 >= 4) {// if is other capture
		this->zobrist_key ^= transposition_table::piece_keys[(side_to_move) ? 
			m->get_captured_type() : m->get_captured_type() + BLACK_PAWN][target];
	}
	
	uint8_t castling_difference = (this->castling_rights ^ this->game_history.back().castling_rights) & 15U;
	if (castling_difference & w_kingside) {
		this->zobrist_key ^= transposition_table::castling_keys[WHITE_KINGSIDE];
	} 
	if (castling_difference & w_queenside) {
		this->zobrist_key ^= transposition_table::castling_keys[WHITE_QUEENSIDE];
	}
	if (castling_difference & b_kingside) {
		this->zobrist_key ^= transposition_table::castling_keys[BLACK_KINGSIDE];
	}
	if (castling_difference & b_queenside) {
		this->zobrist_key ^= transposition_table::castling_keys[BLACK_QUEENSIDE];
	}
	this->zobrist_key ^= transposition_table::side_key;
}


/// <summary>
///	Returns whether the given square is attacked by the given side.
/// </summary>
/// <param name="square"></param>
/// <param name="side_to_move"></param>
/// <returns></returns>
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

void bitboard::print_board()
{
	unsigned long index = 0;
	std::string pieces_str[64] = { };
	_BitScanForward64(&index, this->bbs[KING][WHITE]);
	pieces_str[index] = "K";
	_BitScanForward64(&index, this->bbs[KING][BLACK]);
	pieces_str[index] = "k";
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
	std::cout << "ep-target-square: " << ep_target_square << std::endl;
	std::cout << "Hash: " << zobrist_key << std::endl;
}



void bitboard::make_move(bit_move* m)
{
	// save the current game state in the game history vector
	this->game_history.emplace_back(0ULL, this->castling_rights, this->ep_target_square, this->fifty_move_rule_counter, *m);
	// extract move details
	uint8_t origin = m->get_origin();
	uint8_t target = m->get_target();
	uint8_t flags = m->get_flags();
	uint8_t piece_type = m->get_piece_type();
	uint8_t captured_type = m->get_captured_type();
	uint8_t captured_piece = pieces[target];
	// convert origin & target to bitmasks
	uint64_t origin_bit = bitboard_util::set_bit(origin);
	uint64_t target_bit = bitboard_util::set_bit(target);

	this->fifty_move_rule_counter++;

	// Remove captured pieces
	if (captured_type != EMPTY) {
		this->fifty_move_rule_counter = 0;
		switch (captured_type)
		{
		case PAWN:
			if (flags == bit_move::ep_capture) {
				bbs[PAWN][!side_to_move] ^= (side_to_move) ? target_bit << 8 : target_bit >> 8;
				types[(side_to_move) ? target + 8 : target - 8] = EMPTY;
				pieces[(side_to_move) ? target + 8 : target - 8] = EMPTY_PIECE;
			}
			else {
				bbs[PAWN][!side_to_move] ^= target_bit;
				types[target] = EMPTY;
				pieces[target] = EMPTY_PIECE;
			}
			break;
		case ROOK:
			bbs[ROOK][!side_to_move] ^= target_bit;
			types[target] = EMPTY;
			pieces[target] = EMPTY_PIECE;
			if (target == 0) {
				this->castling_rights &= ~w_queenside;
			}
			else if (target == 7) {
				this->castling_rights &= ~w_kingside;
			}
			else if (target == 56) {
				this->castling_rights &= ~b_queenside;
			}
			else if (target == 63) {
				this->castling_rights &= ~b_kingside;
			}
			break;
		default:
			bbs[captured_type][!side_to_move] ^= target_bit;
			types[target] = EMPTY;
			pieces[target] = EMPTY_PIECE;
			break;
		}
	}
	if (flags == bit_move::double_pawn_push) {
		this->ep_target_square = target - ((side_to_move) ? -8 : 8);
		this->fifty_move_rule_counter = 0;
	}
	else {
		this->ep_target_square = -1;
	}
	if (piece_type == PAWN && flags < 8) { // if is pawn move and not promotion
		this->fifty_move_rule_counter = 0;
	}
	else if (piece_type == PAWN && flags >= 8) { // if is promotion
		this->fifty_move_rule_counter = 0;
		bbs[PAWN][side_to_move] ^= origin_bit;
		types[origin] = EMPTY;
		pieces[origin] = EMPTY_PIECE;
		if (flags == bit_move::queen_promotion || flags == bit_move::queen_capture_promotion) {
			bbs[QUEEN][side_to_move] ^= target_bit;
			types[target] = QUEEN;
			pieces[target] = (side_to_move) ? BLACK_QUEEN : WHITE_QUEEN;
		}
		else if (flags == bit_move::rook_promotion || flags == bit_move::rook_capture_promotion) {
			bbs[ROOK][side_to_move] ^= target_bit;
			types[target] = ROOK;
			pieces[target] = (side_to_move) ? BLACK_ROOK : WHITE_ROOK;
		}
		else if (flags == bit_move::bishop_promotion || flags == bit_move::bishop_capture_promotion) {
			bbs[BISHOP][side_to_move] ^= target_bit;
			types[target] = BISHOP;
			pieces[target] = (side_to_move) ? BLACK_BISHOP : WHITE_BISHOP;
		}
		else if (flags == bit_move::knight_promotion || flags == bit_move::knight_capture_promotion) {
			bbs[KNIGHT][side_to_move] ^= target_bit;
			types[target] = KNIGHT;
			pieces[target] = (side_to_move) ? BLACK_KNIGHT : WHITE_KNIGHT;
		}
	}
	else if (piece_type == KING && flags == bit_move::kingside_castle) { // if is castle
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 1);
		types[origin + 1] = ROOK;
		pieces[origin + 1] = (side_to_move) ? BLACK_ROOK : WHITE_ROOK;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 3);
		types[origin + 3] = EMPTY;
		pieces[origin + 3] = EMPTY_PIECE;
		// update castling rights
		this->castling_rights &= ~((side_to_move) ? b_kingside | b_queenside : w_kingside | w_queenside);
	}
	else if (piece_type == KING && flags == bit_move::queenside_castle) { // if is castle
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 1);
		types[origin - 1] = ROOK;
		pieces[origin - 1] = (side_to_move) ? BLACK_ROOK : WHITE_ROOK;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 4);
		types[origin - 4] = ROOK;
		pieces[origin - 4] = (side_to_move) ? BLACK_ROOK : WHITE_ROOK;
		// update castling rights
		this->castling_rights &= ~((side_to_move) ? b_queenside | b_kingside : w_queenside | w_kingside);
	}
	else if (piece_type == KING) { // if is king move
		// if king has not moved update castling rights
		if (origin == 4) {
			castling_rights &= ~(w_kingside | w_queenside);
		}
		else if (origin == 60) {
			castling_rights &= ~(b_kingside | b_queenside);
		}
	}
	else if (piece_type == ROOK) { // if is rook move
		// if rook has not moved update castling rights
		if (origin == 7) {
			this->castling_rights &= ~w_kingside;
		}
		else if (origin == 0) {
			this->castling_rights &= ~w_queenside;
		}
		if (origin == 63) {
			this->castling_rights &= ~b_kingside;
		}
		else if (origin == 56) {
			this->castling_rights &= ~b_queenside;
		}
	}

	if (flags < 8) { // if move is not promotion
		this->bbs[piece_type][side_to_move] ^= origin_bit;
		this->types[origin] = EMPTY;
		this->pieces[origin] = EMPTY_PIECE;
		this->bbs[piece_type][side_to_move] ^= target_bit;
		this->types[target] = piece_type;
		this->pieces[target] = (side_to_move) ? piece_type + BLACK_PAWN : piece_type;
	}
	update_zobrist_key(m);
	this->side_to_move = !side_to_move;
	full_move_clock += side_to_move;
	occupancy[0] = bbs[PAWN][0] | bbs[KNIGHT][0] | bbs[BISHOP][0] | bbs[ROOK][0] | bbs[QUEEN][0] | bbs[KING][0];
	occupancy[1] = bbs[PAWN][1] | bbs[KNIGHT][1] | bbs[BISHOP][1] | bbs[ROOK][1] | bbs[QUEEN][1] | bbs[KING][1];
	occupancy[2] = occupancy[0] | occupancy[1];
	
}

void bitboard::unmake_move()
{
	// pop previous board state from game history vector
	
	board_state prev_board_state = game_history.back();
	

	// restore previous board state
	this->full_move_clock -= side_to_move;
	this->side_to_move = !side_to_move;
	this->fifty_move_rule_counter = prev_board_state.fifty_move_counter;
	this->ep_target_square = prev_board_state.en_passant_target_square;
	
	bit_move prev_move = prev_board_state.last_move;
	uint8_t origin = prev_move.get_origin();
	uint8_t target = prev_move.get_target();
	uint8_t piece_type = prev_move.get_piece_type();
	uint8_t captured_type = prev_move.get_captured_type();
	uint8_t flags = prev_move.get_flags();
	update_zobrist_key(&prev_move);
	this->castling_rights = prev_board_state.castling_rights;
	
	game_history.pop_back();
	// restore captured piece on the board representation

	uint64_t origin_bit = bitboard_util::set_bit(origin);
	uint64_t target_bit = bitboard_util::set_bit(target);

	if (captured_type != EMPTY) { // if is capture
		if (flags == bit_move::ep_capture) {
			bbs[PAWN][!side_to_move] ^= (side_to_move) ? target_bit << 8 : target_bit >> 8;
			types[(side_to_move) ? target + 8 : target - 8] = PAWN;
			pieces[(side_to_move) ? target + 8 : target - 8] = (side_to_move) ? BLACK_PAWN : WHITE_PAWN;
			types[target] = EMPTY;
			pieces[target] = EMPTY_PIECE;
		}
		else {
			bbs[captured_type][!side_to_move] ^= target_bit;
			types[target] = captured_type;
			pieces[target] = (side_to_move) ? captured_type : captured_type + BLACK_PAWN;
		}
	}

	if (flags == bit_move::kingside_castle) {
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 1);
		types[origin + 1] = EMPTY;
		pieces[origin + 1] = EMPTY_PIECE;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 3);
		types[origin + 3] = ROOK;
		pieces[origin + 3] = (side_to_move) ? BLACK_ROOK : WHITE_ROOK;
	}
	else if (flags == bit_move::queenside_castle) {
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 1);
		types[origin - 1] = EMPTY;
		pieces[origin - 1] = EMPTY_PIECE;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 4);
		types[origin - 4] = ROOK;
		pieces[origin - 4] = (side_to_move) ? BLACK_ROOK : WHITE_ROOK;
	}
	else if (flags >= 8) { // if move is promotion
		int promotion_type = (flags % 4) + 1;
		bbs[PAWN][side_to_move] ^= origin_bit;
		types[origin] = PAWN;
		pieces[origin] = (side_to_move) ? BLACK_PAWN : WHITE_PAWN;
		bbs[promotion_type][side_to_move] ^= target_bit;
		types[target] = captured_type;
		pieces[target] = (side_to_move) ? promotion_type + BLACK_PAWN : promotion_type;
	}
	if (flags < 8) { // if move is not promotion
		bbs[piece_type][side_to_move] ^= origin_bit;
		bbs[piece_type][side_to_move] ^= target_bit;
		types[origin] = piece_type;
		pieces[origin] = (side_to_move) ? piece_type + BLACK_PAWN : piece_type;
		if (captured_type == EMPTY || captured_type == 15) {
			types[target] = EMPTY;
			pieces[target] = EMPTY_PIECE;
		}
	}



	occupancy[0] = bbs[PAWN][0] | bbs[KNIGHT][0] | bbs[BISHOP][0] | bbs[ROOK][0] | bbs[QUEEN][0] | bbs[KING][0];
	occupancy[1] = bbs[PAWN][1] | bbs[KNIGHT][1] | bbs[BISHOP][1] | bbs[ROOK][1] | bbs[QUEEN][1] | bbs[KING][1];
	occupancy[2] = occupancy[0] | occupancy[1];
}

uint8_t bitboard::piece_type_from_index(unsigned long i)
{
	/*uint64_t mask = (1ULL << i);
	bool allegiance = (mask & pieces[0]) == 0;
	if ((bbs[PAWN][allegiance] & mask) != 0ULL) {
		return PAWN;
	}
	if ((bbs[KNIGHT][allegiance] & mask) != 0ULL) {
		return bitboard_util::knight;
	}
	if ((bbs[BISHOP][allegiance] & mask) != 0ULL) {
		return bitboard_util::bishop;
	}
	if ((bbs[ROOK][allegiance] & mask) != 0ULL) {
		return ROOK;
	}
	if ((bbs[QUEEN][allegiance] & mask) != 0ULL) {
		return bitboard_util::queen;
	}
	if ((bbs[KING][allegiance] & mask) != 0ULL) {
		return KING;
	}
	return EMPTY;*/
	return types[i];
}

