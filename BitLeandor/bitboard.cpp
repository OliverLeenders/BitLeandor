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
					this->pawns[0] |= bitboard_util::set_bit(board_index);
					break;
				case 'p':
					this->pawns[1] |= bitboard_util::set_bit(board_index);
					break;
				case 'N':
					this->knights[0] |= bitboard_util::set_bit(board_index);
					break;
				case 'n':
					this->knights[1] |= bitboard_util::set_bit(board_index);
					break;
				case 'B':
					this->bishops[0] |= bitboard_util::set_bit(board_index);
					break;
				case 'b':
					this->bishops[1] |= bitboard_util::set_bit(board_index);
					break;
				case 'R':
					this->rooks[0] |= bitboard_util::set_bit(board_index);
					break;
				case 'r':
					this->rooks[1] |= bitboard_util::set_bit(board_index);
					break;
				case 'Q':
					this->queens[0] |= bitboard_util::set_bit(board_index);
					break;
				case 'q':
					this->queens[1] |= bitboard_util::set_bit(board_index);
					break;
				case 'K':
					this->kings[0] |= bitboard_util::set_bit(board_index);
					break;
				case 'k':
					this->kings[1] |= bitboard_util::set_bit(board_index);
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
	
	this->pieces[0] = this->pawns[0] | this->knights[0] | this->bishops[0] | this->rooks[0] | this->queens[0] | this->kings[0];
	this->pieces[1] = this->pawns[1] | this->knights[1] | this->bishops[1] | this->rooks[1] | this->queens[1] | this->kings[1];
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

	std::string fen_ep_target_square = fen_split[3];
	if (fen_ep_target_square == "-") {
		this->ep_target_square = -1;
	}
	else {
		this->ep_target_square = 8 * char_to_rank(fen_ep_target_square.at(0)) + char_to_file(fen_ep_target_square.at(1));
	}

	std::string fen_fifty_move_clock = fen_split[4];

	return;
}

int bitboard::char_to_rank(char c) {
	return c - '0' - 1;
}
int bitboard::char_to_file(char c) {
	return c - 'a';
}



bool bitboard::is_square_attacked(int square, bool side_to_move) {
	if (attacks::pawn_attacks[!side_to_move][square] & pawns[side_to_move]) {
		return 1;
	}
	if (attacks::knight_attacks[square] & knights[side_to_move]) {
		return 1;
	}
	if (attacks::get_bishop_attacks(square, pieces[0] | pieces[1]) & (bishops[side_to_move] | queens[side_to_move])) {
		return 1;
	}
	if (attacks::get_rook_attacks(square, pieces[0] | pieces[1]) & (rooks[side_to_move] | queens[side_to_move])) {
		return 1;
	}
	if (attacks::king_attacks[square] & kings[side_to_move]) {
		return 1;
	}
	return 0;
}



bool bitboard::is_legal(bit_move* m)
{

}

void bitboard::make_move(bit_move* m)
{
	// save the current game state in the game history vector
	this->game_history.emplace_back(0ULL, this->castling_rights, this->ep_target_square, this->fifty_move_rule_counter, m);
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
			pawns[!side_to_move] ^= target_bit;
			break;
		case bitboard_util::knight:
			knights[!side_to_move] ^= target_bit;
			break;
		case bitboard_util::bishop:
			bishops[!side_to_move] ^= target_bit;
			break;
		case bitboard_util::rook:
			rooks[!side_to_move] ^= target_bit;
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
			queens[!side_to_move] ^= target_bit;
			break;
		case bitboard_util::king:
			kings[!side_to_move] ^= target_bit;
			break;
		default:
			break;
		}
	}
	if (flags == bit_move::double_pawn_push) {
		this->ep_target_square = target - ((side_to_move) ? 8 : -8);
		this->fifty_move_rule_counter = 0;
	}
	else {
		this->ep_target_square = -1;
	}
	if (piece_type == bitboard_util::pawn && flags < 8) { // if is pawn move and not promotion
		pawns[side_to_move] ^= origin_bit;
		pawns[side_to_move] ^= target_bit;
		this->fifty_move_rule_counter = 0;
	}
	else if (piece_type == bitboard_util::pawn && flags >= 8) { // if is promotion
		this->fifty_move_rule_counter = 0;
		pawns[side_to_move] ^= origin_bit;
		if (flags == bitboard_util::queen) {
			queens[side_to_move] ^= target_bit;
		}
		else if (flags == bitboard_util::rook) {
			rooks[side_to_move] ^= target_bit;
		}
		else if (flags == bitboard_util::bishop) {
			bishops[side_to_move] ^= target_bit;
		}
		else if (flags == bitboard_util::knight) {
			knights[side_to_move] ^= target_bit;
		}
	}
	else if (piece_type == bitboard_util::king && flags == bit_move::kingside_castle) { // if is castle
		kings[side_to_move] ^= origin_bit;
		kings[side_to_move] ^= target_bit;
		rooks[side_to_move] ^= bitboard_util::set_bit(origin + 1);
		rooks[side_to_move] ^= bitboard_util::set_bit(origin + 3);
		// update castling rights
		this->castling_rights &= ~((side_to_move) ? b_kingside : w_kingside);
	}
	else if (piece_type == bitboard_util::king && flags == bit_move::queenside_castle) { // if is castle
		kings[side_to_move] ^= origin_bit;
		kings[side_to_move] ^= target_bit;
		rooks[side_to_move] ^= bitboard_util::set_bit(origin - 1);
		rooks[side_to_move] ^= bitboard_util::set_bit(origin - 4);
		// update castling rights
		this->castling_rights &= ~((side_to_move) ? b_queenside : w_queenside);
	}
	else if (piece_type == bitboard_util::king) { // if is king move
		kings[side_to_move] ^= origin_bit;
		kings[side_to_move] ^= target_bit;
		// if king has not moved update castling rights
		if (origin == 4) {
			castling_rights &= ~(w_kingside | w_queenside);
		}
		else if (origin == 60) {
			castling_rights &= ~(b_kingside | b_queenside);
		}
	}
	else if (piece_type == bitboard_util::queen) { // if is queen move
		queens[side_to_move] ^= origin_bit;
		queens[side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::rook) { // if is rook move
		rooks[side_to_move] ^= origin_bit;
		rooks[side_to_move] ^= target_bit;
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
		bishops[side_to_move] ^= origin_bit;
		bishops[side_to_move] ^= target_bit;
	}
	else if (piece_type == bitboard_util::knight) { // if is knight move
		knights[side_to_move] ^= origin_bit;
		knights[side_to_move] ^= target_bit;
	}	
	this->side_to_move = !side_to_move;
	full_move_clock += side_to_move;
}

void bitboard::unmake_move()
{
	// pop previous board state from game history vector
	board_state prev_board_state = game_history.back();
	game_history.pop_back();
	// restore previous board state
	this->side_to_move = !side_to_move;
	this->fifty_move_rule_counter = prev_board_state.fifty_move_counter;
	this->castling_rights = prev_board_state.castling_rights;
	// restore captured piece on the board representation

	
}

uint8_t bitboard::piece_type_from_index(unsigned long i)
{
	uint64_t mask = (1ULL << i);
	bool allegiance = (mask & pieces[0]) == 0;
	if ((pawns[allegiance] & mask) != 1ULL) {
		return bitboard_util::pawn;
	}
	if ((knights[allegiance] & mask) != 1ULL) {
		return bitboard_util::knight;
	}
	if ((bishops[allegiance] & mask) != 1ULL) {
		return bitboard_util::bishop;
	}
	if ((rooks[allegiance] & mask) != 1ULL) {
		return bitboard_util::rook;
	}
	if ((queens[allegiance] & mask) != 1ULL) {
		return bitboard_util::queen;
	}
	if ((kings[allegiance] & mask) != 1ULL) {
		return bitboard_util::king;
	}
	return bitboard_util::empty;
}

