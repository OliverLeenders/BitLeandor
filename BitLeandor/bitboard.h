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
	// occupancy bitboards
	uint64_t pieces[3] = { 0 };
	bool side_to_move = 0;
	void pos_from_fen(std::string fen);
	int char_to_rank(char c);
	int char_to_file(char c);
	bool is_square_attacked(int square, bool side_to_move);
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
	bool is_legal(bit_move* m);
	void make_move(bit_move* m);
	void unmake_move();
	uint8_t piece_type_from_index(unsigned long i);
	std::vector<board_state> game_history = {};
	uint16_t fifty_move_rule_counter = 0;
	uint16_t full_move_clock = 1;
};

