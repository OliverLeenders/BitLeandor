#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <ctype.h>
#include "utility.h"
#include "bit_move.h"
#include "board_state.h"
#include "constants.h"
#include "transposition_table.h"

class bitboard
{
public:
	bitboard();

	uint64_t bbs[6][2] = { {0, 0}, {0, 0}, {0, 0} , {0, 0} , {0, 0}, {0, 0} };
	uint8_t types[64] = { EMPTY };
	uint8_t pieces[64] = { EMPTY_PIECE };

	// occupancy bitboards
	uint64_t occupancy[3] = { 0ULL };
	uint64_t zobrist_key = 0ULL;
	uint64_t pawn_hash_key = 0ULL;
	bool side_to_move = 0;
	void pos_from_fen(std::string fen);
	std::string pos_to_fen();
	int char_to_rank(char c);
	int char_to_file(char c);
	int piece_to_char(uint8_t piece);
	bool is_square_attacked(int square, bool side_to_move);
	
	template<bool side_to_move>
	bool pawns_before_back_rank() {
		uint64_t rank = (side_to_move) ? second_rank : seventh_rank;
		uint64_t promotion_candidates = bbs[PAWN][side_to_move] & rank;
		return promotion_candidates != 0ULL;
	}
	
	uint8_t king_positions[2] = { 0U, 0U };

	void print_board();
	~bitboard();
	char castling_rights = 0;
	const enum castling_rights_e
	{
		w_kingside = 1,
		w_queenside = 2,
		b_kingside = 4,
		b_queenside = 8
	};
	uint64_t ep_target_square = 0ULL;

	bool is_sane();

	// templated is legal function needs to be in header-file for some stupid c++ reason...
	template<bool was_generated>
	bool is_legal(bit_move* m)
	{
		const uint8_t flags = m->get_flags();
		const uint8_t origin = m->get_origin();
		const uint8_t target = m->get_target();
		const uint8_t type = m->get_piece_type();
		const uint8_t piece = (side_to_move) ? type + BLACK_PAWN : type;
		const uint8_t captured_type = m->get_captured_type();
		const uint8_t captured_piece = (side_to_move) ? captured_type : captured_type + BLACK_PAWN;
		
		// if the move was not generated we need to check
		// the availability of a piece which is able to move 
		// to that square (and more) stil incomplete -- does
		// not check vacancy of sliding piece rays

		if (!was_generated) {
			if (origin == target) {
				return false;
			}
			if (piece != pieces[origin]) {
				return false;
			}
			if (flags == bit_move::quiet_move) {
				if (pieces[target] != EMPTY_PIECE) {
					return false;
				}
			}
			else if (flags == bit_move::capture || flags >= bit_move::knight_capture_promotion) {
				if (captured_piece != pieces[target]) { // this one is critical
					return false;
				}
			}
			else if (flags == bit_move::ep_capture) {
				if (target != ep_target_square) {
					return false;
				}
			}
			else if (flags == bit_move::double_pawn_push) {
				if (pieces[target] != EMPTY_PIECE || pieces[target - ((side_to_move) ? -8 : 8)] != EMPTY_PIECE) {
					return false;
				}
			}
			else if (flags == bit_move::kingside_castle) {
				// if we have no castling right
				if (!(this->castling_rights & ((side_to_move) ? b_kingside : w_kingside))) {
					return false;
				} 
				// if the squares are occupied
				if (pieces[origin + 1] != EMPTY_PIECE || pieces[origin + 2] != EMPTY_PIECE) {
					return false;
				}
			}
			else if (flags == bit_move::queenside_castle) {
				// if we have no castling right
				if (!(this->castling_rights & ((side_to_move) ? b_queenside : w_queenside))) {
					return false;
				}
				// if the squares are occupied
				if (pieces[origin - 1] != EMPTY_PIECE || pieces[origin - 2] != EMPTY_PIECE || pieces[origin - 3] != EMPTY_PIECE) {
					return false;
				}
			}
			if (types[origin] >= BISHOP && types[origin] <= QUEEN) {
				if ((attacks::squares_between[origin][target] & occupancy[2]) != 0ULL) {
					/*std::cout << std::endl;
					print_board();
					print_bitboard(attacks::squares_between[origin][target]);
					std::cout << "Move: " << bit_move::to_string(*m) << std::endl;
					*/
					return false;
				}

			}
		}
		if (flags == bit_move::queenside_castle) {
			if (is_square_attacked(origin, !side_to_move) || is_square_attacked(origin - 1, !side_to_move) || is_square_attacked(origin - 2, !side_to_move)) {
				return false;
			}
		}
		else if (flags == bit_move::kingside_castle) {
			if (is_square_attacked(origin, !side_to_move) || is_square_attacked(origin + 1, !side_to_move) || is_square_attacked(origin + 2, !side_to_move)) {
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
	void make_move(bit_move* m);
	void unmake_move();
	template<bool> void place_piece(uint8_t piece, uint8_t target);
	template<bool> void unset_piece(uint8_t target);
	template<bool> void replace_piece(uint8_t piece, uint8_t target);
	void make_null_move();
	void unmake_null_move();
	uint8_t piece_type_from_index(unsigned long i);
	std::vector<board_state> game_history = {};
	uint16_t fifty_move_rule_counter = 0;
	uint16_t full_move_clock = 1;
};

