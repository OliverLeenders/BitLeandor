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

	if (fen_split.size() < 5) {
		std::cout << "fen string too short" << std::endl;
		return;
	}
	std::string fen_pos = fen_split[0];
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
					break;
				case 'p':
					this->bbs[PAWN][1] |= bitboard_util::set_bit(board_index);
					break;
				case 'N':
					this->bbs[KNIGHT][0] |= bitboard_util::set_bit(board_index);
					break;
				case 'n':
					this->bbs[KNIGHT][1] |= bitboard_util::set_bit(board_index);
					break;
				case 'B':
					this->bbs[BISHOP][0] |= bitboard_util::set_bit(board_index);
					break;
				case 'b':
					this->bbs[BISHOP][1] |= bitboard_util::set_bit(board_index);
					break;
				case 'R':
					this->bbs[ROOK][0] |= bitboard_util::set_bit(board_index);
					break;
				case 'r':
					this->bbs[ROOK][1] |= bitboard_util::set_bit(board_index);
					break;
				case 'Q':
					this->bbs[QUEEN][0] |= bitboard_util::set_bit(board_index);
					break;
				case 'q':
					this->bbs[QUEEN][1] |= bitboard_util::set_bit(board_index);
					break;
				case 'K':
					this->bbs[KING][0] |= bitboard_util::set_bit(board_index);
					break;
				case 'k':
					this->bbs[KING][1] |= bitboard_util::set_bit(board_index);
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

	this->pieces[0] = this->bbs[PAWN][0] | this->bbs[KNIGHT][0] | this->bbs[BISHOP][0] | this->bbs[ROOK][0] | this->bbs[QUEEN][0] | this->bbs[KING][0];
	this->pieces[1] = this->bbs[PAWN][1] | this->bbs[KNIGHT][1] | this->bbs[BISHOP][1] | this->bbs[ROOK][1] | this->bbs[QUEEN][1] | this->bbs[KING][1];
	this->pieces[2] = this->pieces[0] | this->pieces[1];

	std::string fen_side_to_move = fen_split[1];
	this->side_to_move = fen_side_to_move == "b";

	std::string fen_castling_rights = fen_split[2];
	for (int i = 0; i < fen_castling_rights.size(); i++) {
		char c = fen_castling_rights.at(i);
		switch (c)
		{
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
	}
	std::cout << "ep_target_square: " << this->ep_target_square << std::endl;
	std::string fen_fifty_move_clock = fen_split[4];

	return;
}

int bitboard::char_to_rank(char c) {
	return c - '0' - 1;
}
int bitboard::char_to_file(char c) {
	return c - 'a';
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
	if (attacks::get_bishop_attacks(square, pieces[0] | pieces[1]) & (bbs[BISHOP][side_to_move] | bbs[QUEEN][side_to_move])) {
		return 1;
	}
	if (attacks::get_rook_attacks(square, pieces[0] | pieces[1]) & (bbs[ROOK][side_to_move] | bbs[QUEEN][side_to_move])) {
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
	_BitScanForward64(&index, this->bbs[KING][0]);
	pieces_str[index] = "K";
	_BitScanForward64(&index, this->bbs[KING][1]);
	pieces_str[index] = "k";
	uint64_t w_queens = this->bbs[QUEEN][0];
	while (w_queens != 0ULL) {
		_BitScanForward64(&index, w_queens);
		pieces_str[index] = "Q";
		w_queens &= w_queens - 1;
	}
	uint64_t b_queens = this->bbs[QUEEN][1];
	while (b_queens != 0ULL) {
		_BitScanForward64(&index, b_queens);
		pieces_str[index] = "q";
		b_queens &= b_queens - 1;
	}
	uint64_t w_rooks = this->bbs[ROOK][0];
	while (w_rooks != 0ULL) {
		_BitScanForward64(&index, w_rooks);
		pieces_str[index] = "R";
		w_rooks &= w_rooks - 1;
	}
	uint64_t b_rooks = this->bbs[ROOK][1];
	while (b_rooks != 0ULL) {
		_BitScanForward64(&index, b_rooks);
		pieces_str[index] = "r";
		b_rooks &= b_rooks - 1;
	}
	uint64_t w_bishops = this->bbs[BISHOP][0];
	while (w_bishops != 0ULL) {
		_BitScanForward64(&index, w_bishops);
		pieces_str[index] = "B";
		w_bishops &= w_bishops - 1;
	}
	uint64_t b_bishops = this->bbs[BISHOP][1];
	while (b_bishops != 0ULL) {
		_BitScanForward64(&index, b_bishops);
		pieces_str[index] = "b";
		b_bishops &= b_bishops - 1;
	}
	uint64_t w_knights = this->bbs[KNIGHT][0];
	while (w_knights != 0ULL) {
		_BitScanForward64(&index, w_knights);
		pieces_str[index] = "N";
		w_knights &= w_knights - 1;
	}
	uint64_t b_knights = this->bbs[KNIGHT][1];
	while (b_knights != 0ULL) {
		_BitScanForward64(&index, b_knights);
		pieces_str[index] = "n";
		b_knights &= b_knights - 1;
	}
	uint64_t w_pawns = this->bbs[PAWN][0];
	while (w_pawns != 0ULL) {
		_BitScanForward64(&index, w_pawns);
		pieces_str[index] = "P";
		w_pawns &= w_pawns - 1;
	}
	uint64_t b_pawns = this->bbs[PAWN][1];
	while (b_pawns != 0ULL) {
		_BitScanForward64(&index, b_pawns);
		pieces_str[index] = "p";
		b_pawns &= b_pawns - 1;
	}
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
	for (board_state s : game_history) {
		std::cout << bit_move::to_string(s.last_move) << " ";
	}
	std::cout << std::endl;
	std::cout << "ep-target-square: " << ep_target_square << std::endl;
}



void bitboard::make_move(bit_move* m)
{
	// save the current game state in the game history vector
	this->game_history.emplace_back(0ULL, this->castling_rights, this->ep_target_square, this->fifty_move_rule_counter, *m);
	// extract move details
	uint16_t origin = m->get_origin();
	uint16_t target = m->get_target();
	uint8_t flags = m->get_flags();
	uint8_t piece_type = m->get_piece_type();
	uint8_t captured_type = m->get_captured_type();
	// convert origin & target to bitmasks
	uint64_t origin_bit = bitboard_util::set_bit(origin);
	uint64_t target_bit = bitboard_util::set_bit(target);

	this->fifty_move_rule_counter++;

	if (captured_type != bitboard_util::empty) {
		this->fifty_move_rule_counter = 0;
		switch (captured_type)
		{
		case bitboard_util::pawn:
			if (flags != bit_move::ep_capture) {
				bbs[PAWN][!side_to_move] ^= target_bit;
				//	print_board();
			}
			else {
				// std::cout << "\nen_passant" << std::endl;
				// print_board();
				bbs[PAWN][!side_to_move] ^= (side_to_move) ? target_bit << 8: target_bit >> 8;
				//print_board();
				// std::cout << ep_target_square << std::endl;
			}
			break;
		case bitboard_util::knight:
			bbs[KNIGHT][!side_to_move] ^= target_bit;
			break;
		case bitboard_util::bishop:
			bbs[BISHOP][!side_to_move] ^= target_bit;
			break;
		case bitboard_util::rook:
			bbs[ROOK][!side_to_move] ^= target_bit;
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
		case bitboard_util::queen:
			bbs[QUEEN][!side_to_move] ^= target_bit;
			break;
		case bitboard_util::king:
			bbs[KING][!side_to_move] ^= target_bit;
			break;
		default:
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
	if (piece_type == bitboard_util::pawn && flags < 8) { // if is pawn move and not promotion
		// std::cout << "pawn move" << std::endl;

		bbs[PAWN][side_to_move] ^= origin_bit;
		bbs[PAWN][side_to_move] ^= target_bit;
		this->fifty_move_rule_counter = 0;
		//bitboard_util::print_bitboard(bbs[PAWN][side_to_move]);
		//bitboard_util::print_bitboard(origin_bit);
		//bitboard_util::print_bitboard(target_bit);
	}
	else if (piece_type == bitboard_util::pawn && flags >= 8) { // if is promotion
		this->fifty_move_rule_counter = 0;
		bbs[PAWN][side_to_move] ^= origin_bit;
		if (flags == bit_move::queen_promotion || flags == bit_move::queen_capture_promotion) {
			bbs[QUEEN][side_to_move] ^= target_bit;
		}
		else if (flags == bit_move::rook_promotion || flags == bit_move::rook_capture_promotion) {
			bbs[ROOK][side_to_move] ^= target_bit;
		}
		else if (flags == bit_move::bishop_promotion || flags == bit_move::bishop_capture_promotion) {
			bbs[BISHOP][side_to_move] ^= target_bit;
		}
		else if (flags == bit_move::knight_promotion || flags == bit_move::knight_capture_promotion) {
			bbs[KNIGHT][side_to_move] ^= target_bit;
		}
	}
	else if (piece_type == bitboard_util::king && flags == bit_move::kingside_castle) { // if is castle
		bbs[KING][side_to_move] ^= origin_bit;
		bbs[KING][side_to_move] ^= target_bit;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 1);
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 3);
		// update castling rights
		this->castling_rights &= ~((side_to_move) ? b_kingside | b_queenside : w_kingside | w_queenside);
	}
	else if (piece_type == bitboard_util::king && flags == bit_move::queenside_castle) { // if is castle
		bbs[KING][side_to_move] ^= origin_bit;
		bbs[KING][side_to_move] ^= target_bit;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 1);
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 4);
		// update castling rights
		this->castling_rights &= ~((side_to_move) ? b_queenside | b_kingside : w_queenside | w_kingside);
	}
	else if (piece_type == bitboard_util::king) { // if is king move
		bbs[KING][side_to_move] ^= origin_bit;
		bbs[KING][side_to_move] ^= target_bit;
		// if king has not moved update castling rights
		if (origin == 4) {
			castling_rights &= ~(w_kingside | w_queenside);
		}
		else if (origin == 60) {
			castling_rights &= ~(b_kingside | b_queenside);
		}
	}
	else if (piece_type == bitboard_util::queen) { // if is queen move
		bbs[QUEEN][side_to_move] ^= origin_bit;
		bbs[QUEEN][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::rook) { // if is rook move
		bbs[ROOK][side_to_move] ^= origin_bit;
		bbs[ROOK][side_to_move] ^= target_bit;
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
	else if (piece_type == bitboard_util::bishop) { // if is bishop move
		bbs[BISHOP][side_to_move] ^= origin_bit;
		bbs[BISHOP][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::knight) { // if is knight move
		bbs[KNIGHT][side_to_move] ^= origin_bit;
		bbs[KNIGHT][side_to_move] ^= target_bit;
	}
	this->side_to_move = !side_to_move;
	full_move_clock += side_to_move;
	pieces[0] = bbs[PAWN][0] | bbs[KNIGHT][0] | bbs[BISHOP][0] | bbs[ROOK][0] | bbs[QUEEN][0] | bbs[KING][0];
	pieces[1] = bbs[PAWN][1] | bbs[KNIGHT][1] | bbs[BISHOP][1] | bbs[ROOK][1] | bbs[QUEEN][1] | bbs[KING][1];
	pieces[2] = pieces[0] | pieces[1];
	unsigned long popcnt = 0;
	if (__popcnt64(pieces[2]) > 32) {
		std::cout << bit_move::to_string(*m) << " (" << (int)m->get_flags() << ") error" << std::endl;
		print_board();
	}
}

void bitboard::unmake_move()
{
	// pop previous board state from game history vector
	board_state prev_board_state = game_history.back();
	game_history.pop_back();
	// restore previous board state
	this->full_move_clock -= side_to_move;
	this->side_to_move = !side_to_move;
	this->fifty_move_rule_counter = prev_board_state.fifty_move_counter;
	this->castling_rights = prev_board_state.castling_rights;
	this->ep_target_square = prev_board_state.en_passant_target_square;
	// restore captured piece on the board representation
	bit_move prev_move = prev_board_state.last_move;
	int origin = prev_move.get_origin();
	int target = prev_move.get_target();
	int piece_type = prev_move.get_piece_type();
	int captured_type = prev_move.get_captured_type();
	int flags = prev_move.get_flags();

	uint64_t origin_bit = bitboard_util::set_bit(origin);
	uint64_t target_bit = bitboard_util::set_bit(target);

	if (captured_type != bitboard_util::empty) { // if is capture
		switch (captured_type)
		{
		case bitboard_util::pawn:
			if (flags != bit_move::ep_capture) {
				bbs[PAWN][!side_to_move] ^= target_bit;
			}
			else {
				bbs[PAWN][!side_to_move] ^= (side_to_move) ? (target_bit << 8) : (target_bit >> 8);
			}
			break;
		case bitboard_util::knight:
			bbs[KNIGHT][!side_to_move] ^= target_bit;
			break;
		case bitboard_util::bishop:
			bbs[BISHOP][!side_to_move] ^= target_bit;
			break;
		case bitboard_util::rook:
			bbs[ROOK][!side_to_move] ^= target_bit;
			break;
		case bitboard_util::queen:
			bbs[QUEEN][!side_to_move] ^= target_bit;
			break;
		default:
			break;
		}
	}

	if (flags == bit_move::kingside_castle) {
		bbs[KING][side_to_move] ^= origin_bit;
		bbs[KING][side_to_move] ^= target_bit;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 1);
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin + 3);
	}
	else if (flags == bit_move::queenside_castle) {
		bbs[KING][side_to_move] ^= origin_bit;
		bbs[KING][side_to_move] ^= target_bit;
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 1);
		bbs[ROOK][side_to_move] ^= bitboard_util::set_bit(origin - 4);
	}
	else if (piece_type == bitboard_util::king) {
		bbs[KING][side_to_move] ^= origin_bit;
		bbs[KING][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::queen) {
		bbs[QUEEN][side_to_move] ^= origin_bit;
		bbs[QUEEN][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::rook) {
		bbs[ROOK][side_to_move] ^= origin_bit;
		bbs[ROOK][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::bishop) {
		bbs[BISHOP][side_to_move] ^= origin_bit;
		bbs[BISHOP][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::knight) {
		bbs[KNIGHT][side_to_move] ^= origin_bit;
		bbs[KNIGHT][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::pawn && flags < 8) {
		bbs[PAWN][side_to_move] ^= origin_bit;
		bbs[PAWN][side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::pawn && flags >= 8) {
		uint8_t promotion_type = flags % 4;
		bbs[PAWN][side_to_move] ^= origin_bit;
		switch (flags)
		{
		case bit_move::knight_promotion:
			bbs[KNIGHT][side_to_move] ^= target_bit;
			break;
		case bit_move::bishop_promotion:
			bbs[BISHOP][side_to_move] ^= target_bit;
			break;
		case bit_move::rook_promotion:
			bbs[ROOK][side_to_move] ^= target_bit;
			break;
		case bit_move::queen_promotion:
			bbs[QUEEN][side_to_move] ^= target_bit;
			break;
		case bit_move::knight_capture_promotion:
			bbs[KNIGHT][side_to_move] ^= target_bit;
			break;
		case bit_move::bishop_capture_promotion:
			bbs[BISHOP][side_to_move] ^= target_bit;
			break;
		case bit_move::rook_capture_promotion:
			bbs[ROOK][side_to_move] ^= target_bit;
			break;
		case bit_move::queen_capture_promotion:
			bbs[QUEEN][side_to_move] ^= target_bit;
			break;
		default:
			break;
		}
	}

	pieces[0] = bbs[PAWN][0] | bbs[KNIGHT][0] | bbs[BISHOP][0] | bbs[ROOK][0] | bbs[QUEEN][0] | bbs[KING][0];
	pieces[1] = bbs[PAWN][1] | bbs[KNIGHT][1] | bbs[BISHOP][1] | bbs[ROOK][1] | bbs[QUEEN][1] | bbs[KING][1];
	pieces[2] = pieces[0] | pieces[1];
	if (__popcnt64(pieces[2]) > 32) {
		std::cout << bit_move::to_string(prev_move) << " error" << std::endl;
		bitboard_util::print_bitboard(bbs[PAWN][0]);
		bitboard_util::print_bitboard(bbs[PAWN][1]);
	}
}

uint8_t bitboard::piece_type_from_index(unsigned long i)
{
	uint64_t mask = (1ULL << i);
	bool allegiance = (mask & pieces[0]) == 0;
	if ((bbs[PAWN][allegiance] & mask) != 0ULL) {
		return bitboard_util::pawn;
	}
	if ((bbs[KNIGHT][allegiance] & mask) != 0ULL) {
		return bitboard_util::knight;
	}
	if ((bbs[BISHOP][allegiance] & mask) != 0ULL) {
		return bitboard_util::bishop;
	}
	if ((bbs[ROOK][allegiance] & mask) != 0ULL) {
		return bitboard_util::rook;
	}
	if ((bbs[QUEEN][allegiance] & mask) != 0ULL) {
		return bitboard_util::queen;
	}
	if ((bbs[KING][allegiance] & mask) != 0ULL) {
		return bitboard_util::king;
	}
	return bitboard_util::empty;
}

