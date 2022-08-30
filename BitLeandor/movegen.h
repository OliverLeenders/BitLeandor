#pragma once

#include <list>
#include "bit_move.h"
#include "bitboard_util.h"
#include "bitboard.h"
#include "movelist.h"

class movegen
{
public:
	movegen();
	~movegen();
	
	static void generate_pawn_qmoves(bitboard* b, movelist* l);
	static const int push_offset = 8;
	/// <summary>
/// Generate pawn captures including en-passant captures and capture promotions
/// </summary>
/// <param name="b">the board</param>
/// <param name="l">the movelist to append to</param>
	template<bool side_to_move>
	static void generate_pawn_cmoves(bitboard* b, movelist* l)
	{
		unsigned long origin;
		unsigned long target;
		
		uint64_t pawns = b->bbs[PAWN][side_to_move];
		uint64_t pushes = (side_to_move) ? ((pawns >> push_offset) & ~b->occupancy[2]) : ((pawns << push_offset) & ~b->occupancy[2]);
		uint64_t promotion_rank = (side_to_move) ? bitboard_util::first_rank : bitboard_util::eighth_rank;
		uint64_t promotion_pushes = pushes & promotion_rank;
		int offset = (side_to_move) ? -push_offset : push_offset;
		while (pawns != 0ULL) {
			_BitScanForward64(&origin, pawns);
			uint64_t pawn_attacks = attacks::pawn_attacks[side_to_move][origin] & (b->occupancy[!side_to_move] | (1ULL << b->ep_target_square));
			while (pawn_attacks != 0ULL) {
				_BitScanForward64(&target, pawn_attacks);
				uint8_t capture_type = b->piece_type_from_index(target);
				if (target == b->ep_target_square) {
					l->moves[l->size] = bit_move(origin, target, bit_move::ep_capture, PAWN, PAWN);
					l->size++;
				}
				else if ((1ULL << target) & promotion_rank) {
					l->moves[l->size] = bit_move(origin, target, bit_move::queen_capture_promotion, PAWN, capture_type);
					l->moves[l->size + 1] = bit_move(origin, target, bit_move::rook_capture_promotion, PAWN, capture_type);
					l->moves[l->size + 2] = bit_move(origin, target, bit_move::bishop_capture_promotion, PAWN, capture_type);
					l->moves[l->size + 3] = bit_move(origin, target, bit_move::knight_capture_promotion, PAWN, capture_type);
					l->size += 4;
				}
				else {
					l->moves[l->size] = bit_move(origin, target, bit_move::capture, PAWN, capture_type);
					l->size++;
				}
				pawn_attacks ^= 1ULL << target;
			}
			pawns ^= 1ULL << origin;
		}
		while (promotion_pushes != 0ULL) {
			_BitScanForward64(&target, promotion_pushes);
			l->moves[l->size] = bit_move(target - offset, target, bit_move::queen_promotion, PAWN, EMPTY);
			l->moves[l->size + 1] = bit_move(target - offset, target, bit_move::rook_promotion, PAWN, EMPTY);
			l->moves[l->size + 2] = bit_move(target - offset, target, bit_move::bishop_promotion, PAWN, EMPTY);
			l->moves[l->size + 3] = bit_move(target - offset, target, bit_move::knight_promotion, PAWN, EMPTY);
			l->size += 4;
			promotion_pushes ^= 1ULL << target;
		}
	}

	static void generate_knight_qmoves(bitboard* b, movelist* l);
	static void generate_knight_cmoves(bitboard* b, movelist* l);

	static void generate_bishop_qmoves(bitboard* b, movelist* l);
	static void generate_bishop_cmoves(bitboard* b, movelist* l);

	static void generate_rook_qmoves(bitboard* b, movelist* l);
	static void generate_rook_cmoves(bitboard* b, movelist* l);

	static void generate_queen_qmoves(bitboard* b, movelist* l);
	static void generate_queen_cmoves(bitboard* b, movelist* l);

	static void generate_king_qmoves(bitboard* b, movelist* l);
	static void generate_king_cmoves(bitboard* b, movelist* l);

	static void generate_all_captures(bitboard* b, movelist* l);
	static void generate_all_quiet_moves(bitboard* b, movelist* l);

	static void generate_all_pseudo_legal_moves(bitboard* b, movelist* l);
	
};

