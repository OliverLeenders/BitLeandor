#include "bitboards.h"

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

bitboard::bitboard()
{

}

bitboard::~bitboard()
{
}

