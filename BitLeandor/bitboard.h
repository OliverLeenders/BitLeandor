#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <ctype.h>
#include "utility.h"
#include "bit_move.h"
#include "board_state.h"

class bitboard
{
public:
	bitboard();
	uint64_t pawns[2] = { 0 };
	uint64_t knights[2] = { 0 };
	uint64_t bishops[2] = { 0 };
	uint64_t rooks[2] = { 0 };
	uint64_t queens[2] = { 0 };
	uint64_t kings[2] = { 0 };

	//uint16_t bbs[6][2] = { 0 };

	enum {
		WHITE = 0,
		BLACK = 1
	};

	enum
	{
		PAWN = 0,
		KNIGHT = 1,
		BISHOP = 2,
		ROOK = 3,
		QUEEN = 4,
		KING = 5
	};
	// occupancy bitboards
	uint64_t pieces[3] = { 0 };
	bool side_to_move = 0;
	void pos_from_fen(std::string fen);
	int char_to_rank(char c);
	int char_to_file(char c);
	bool is_square_attacked(int square, bool side_to_move);
	void print_board();
	~bitboard();
	char castling_rights = 0;
	enum castling_rights_e
	{
		w_kingside = 1,
		w_queenside = 2,
		b_kingside = 4,
		b_queenside = 8
	};
	int ep_target_square = -1;

	// templated is legal function needs to be in header-file for some stupid c++ reason...
	template<bool was_generated>
	bool is_legal(bit_move* m)
	{
		uint8_t flags = m->get_flags();
		uint16_t origin = m->get_origin();
		uint16_t target = m->get_target();
		uint64_t origin_mask = 1ULL << origin;
		uint64_t target_mask = 1ULL << target;
		uint8_t piece_type = m->get_piece_type();
		uint8_t captured_type = m->get_captured_type();

		// if the move was not generated we need to check
		// the availability of a piece which is able to move 
		// to that square (and more) 

		if (!was_generated) {
			if ((pieces[side_to_move] & origin_mask) == 0ULL) {
				return false;
			}
			else {
				// find out which piece ...
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
		unsigned long king_pos;
		_BitScanForward64(&king_pos, kings[!side_to_move]);
		if (is_square_attacked(king_pos, side_to_move)) {
			unmake_move();
			return false;
		}
		unmake_move();
		return true;
	}
	void make_move(bit_move* m);
	void unmake_move();
	uint8_t piece_type_from_index(unsigned long i);
	std::vector<board_state> game_history = {};
	uint16_t fifty_move_rule_counter = 0;
	uint16_t full_move_clock = 1;
};

