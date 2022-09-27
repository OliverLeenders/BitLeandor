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

	static const int PUSH_OFFSET = 8;
	static const int NORTH = 8;
	static const int SOUTH = -8;

	template<bool side_to_move>
	static void generate_pawn_qmoves(bitboard* b, movelist* l) {
		uint8_t target;
		int8_t double_push_offset = 16;
		int8_t push_offset = 8;

		uint64_t single_push_rank = (side_to_move) ? sixth_rank : third_rank;
		uint64_t last_rank = (side_to_move) ? first_rank : eighth_rank;

		uint64_t pushes = (side_to_move)
			? ((b->bbs[PAWN][side_to_move] >> push_offset) & ~b->occupancy[2])
			: ((b->bbs[PAWN][side_to_move] << push_offset) & ~b->occupancy[2]);
		uint64_t double_pushes = (side_to_move) 
			? (((pushes & single_push_rank) >> push_offset) & ~b->occupancy[2]) 
			: (((pushes & single_push_rank) << push_offset) & ~b->occupancy[2]);

		pushes = pushes & ~last_rank;

		push_offset = (side_to_move) ? SOUTH : NORTH;
		double_push_offset = 2 * push_offset;


		while (pushes != 0ULL) {
			target = BitScanForward64(pushes);
			l->moves[l->size] = bit_move((uint8_t)target - push_offset, (uint8_t)target, bit_move::quiet_move, PAWN, EMPTY);
			l->size++;
			pushes &= pushes - 1;
		}
		while (double_pushes != 0ULL) {
			target = BitScanForward64(double_pushes);
			l->moves[l->size] = bit_move((uint8_t)target - double_push_offset, (uint8_t)target, bit_move::double_pawn_push, PAWN, EMPTY);
			l->size++;
			double_pushes &= double_pushes - 1;
		}

	}

	/// <summary>
/// Generate pawn captures including en-passant captures and capture promotions
/// </summary>
/// <param name="b">the board</param>
/// <param name="l">the movelist to append to</param>
	template<bool side_to_move>
	static void generate_pawn_cmoves(bitboard* b, movelist* l)
	{
		uint8_t origin;
		uint8_t target;

		uint64_t pawns = b->bbs[PAWN][side_to_move];
		const uint64_t pushes = (side_to_move) ? ((pawns >> PUSH_OFFSET) & ~b->occupancy[2]) : ((pawns << PUSH_OFFSET) & ~b->occupancy[2]);
		uint64_t promotion_rank = (side_to_move) ? first_rank : eighth_rank;
		uint64_t promotion_pushes = pushes & promotion_rank;
		const int8_t offset = (side_to_move) ? SOUTH : NORTH;
		uint64_t pawn_attacks = 0ULL;
		uint8_t capture_type = EMPTY;
		while (pawns != 0ULL) {
			origin = BitScanForward64(pawns);
			if (b->ep_target_square != -1) {
				pawn_attacks = attacks::pawn_attacks[side_to_move][origin] & (b->occupancy[!side_to_move] | (1ULL << b->ep_target_square));
			}
			else {
				pawn_attacks = attacks::pawn_attacks[side_to_move][origin] & (b->occupancy[!side_to_move]);
			}
				while (pawn_attacks != 0ULL) {
					target = BitScanForward64(pawn_attacks);
					capture_type = b->piece_type_from_index(target);
					if (target == b->ep_target_square) {
						l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::ep_capture, PAWN, PAWN);
						l->size++;
					}
					else if ((1ULL << target) & promotion_rank) {
						l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::queen_capture_promotion, PAWN, capture_type);
						l->moves[l->size + 1] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::rook_capture_promotion, PAWN, capture_type);
						l->moves[l->size + 2] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::bishop_capture_promotion, PAWN, capture_type);
						l->moves[l->size + 3] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::knight_capture_promotion, PAWN, capture_type);
						l->size += 4;
					}
					else {
						l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, PAWN, capture_type);
						l->size++;
					}
					pawn_attacks &= pawn_attacks - 1;
				}
			pawns &= origin - 1;
		}
		while (promotion_pushes != 0ULL) {
			target = BitScanForward64(promotion_pushes);
			l->moves[l->size] = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::queen_promotion, PAWN, EMPTY);
			l->moves[l->size + 1] = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::rook_promotion, PAWN, EMPTY);
			l->moves[l->size + 2] = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::bishop_promotion, PAWN, EMPTY);
			l->moves[l->size + 3] = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::knight_promotion, PAWN, EMPTY);
			l->size += 4;
			promotion_pushes &= promotion_pushes - 1;
		}
	}

	template<bool side_to_move>
	static void generate_knight_qmoves(bitboard* b, movelist* l)
	{
		uint64_t knights = b->bbs[KNIGHT][side_to_move];
		uint64_t pieces = b->occupancy[2];
		unsigned long origin;
		unsigned long target;
		uint64_t knight_attacks = 0ULL;
		while (knights != 0ULL) {
			origin = BitScanForward64(knights);
			knight_attacks = attacks::knight_attacks[origin] & (~pieces);
			while (knight_attacks != 0ULL) {
				target = BitScanForward64(knight_attacks);
				l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, KNIGHT, EMPTY);
				l->size++;
				knight_attacks &= knight_attacks - 1;
			}
			knights &= knights - 1;
		}
	}

	template<bool side_to_move>
	static void generate_knight_cmoves(bitboard* b, movelist* l)
	{
		uint64_t knights = b->bbs[KNIGHT][side_to_move];
		uint64_t opp_pieces = b->occupancy[!side_to_move];
		uint8_t origin;
		uint8_t target;
		uint64_t knight_attacks = 0ULL;
		uint8_t capture_type = EMPTY;
		while (knights != 0ULL) {
			origin = BitScanForward64(knights);
			knight_attacks = attacks::knight_attacks[origin] & opp_pieces;
			while (knight_attacks != 0ULL) {
				target = BitScanForward64(knight_attacks);
				capture_type = b->piece_type_from_index(target);
				l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, KNIGHT, capture_type);
				l->size++;
				knight_attacks &= knight_attacks - 1;
			}
			knights &= knights - 1;
		}
	}

	template<bool side_to_move>
	static void generate_bishop_qmoves(bitboard* b, movelist* l)
	{
		uint64_t bishops = b->bbs[BISHOP][side_to_move];
		unsigned long origin;
		unsigned long target;
		uint64_t bishop_attacks = 0ULL;
		while (bishops != 0ULL)
		{
			origin = BitScanForward64(bishops);
			bishop_attacks = attacks::get_bishop_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
			while (bishop_attacks != 0ULL) {
				target = BitScanForward64(bishop_attacks);
				l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, BISHOP, EMPTY);
				l->size++;
				bishop_attacks &= bishop_attacks - 1;
			}
			bishops &= bishops - 1;
		}
	}

	template<bool side_to_move>
	static void generate_bishop_cmoves(bitboard* b, movelist* l)
	{
		uint64_t bishops = b->bbs[BISHOP][side_to_move];
		unsigned long origin;
		unsigned long target;
		uint64_t bishop_attacks = 0ULL;
		uint8_t capture_type = EMPTY;
		while (bishops != 0ULL)
		{
			origin = BitScanForward64(bishops);
			bishop_attacks = attacks::get_bishop_attacks(origin, b->occupancy[2]) & b->occupancy[!b->side_to_move];
			while (bishop_attacks != 0ULL) {
				target = BitScanForward64(bishop_attacks);
				capture_type = b->piece_type_from_index(target);
				l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, BISHOP, capture_type);
				l->size++;
				bishop_attacks &= bishop_attacks - 1;
			}
			bishops &= bishops - 1;
		}
	}

	template<bool side_to_move>
	static void generate_rook_qmoves(bitboard* b, movelist* l)
	{
		uint64_t rooks = b->bbs[ROOK][side_to_move];
		unsigned long origin;
		unsigned long target;
		uint64_t rook_attacks = 0ULL;
		while (rooks != 0ULL)
		{
			origin = BitScanForward64(rooks);
			rook_attacks = attacks::get_rook_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
			while (rook_attacks != 0ULL) {
				target = BitScanForward64(rook_attacks);
				l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, ROOK, EMPTY);
				l->size++;
				rook_attacks &= rook_attacks - 1;
			}
			rooks &= rooks - 1;
		}
	}

	template<bool side_to_move>
	static void generate_rook_cmoves(bitboard* b, movelist* l)
	{
		uint64_t rooks = b->bbs[ROOK][side_to_move];
		uint8_t origin;
		uint8_t target;
		uint64_t rook_attacks = 0ULL;
		uint8_t capture_type = EMPTY;
		while (rooks != 0ULL)
		{
			origin = BitScanForward64(rooks);
			rook_attacks = attacks::get_rook_attacks(origin, b->occupancy[2]) & (b->occupancy[!b->side_to_move]);

			while (rook_attacks != 0ULL) {
				target = BitScanForward64(rook_attacks);
				capture_type = b->piece_type_from_index(target);
				l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, ROOK, capture_type);
				l->size++;
				rook_attacks &= rook_attacks - 1;
			}
			rooks &= rooks - 1;
		}
	}

	static void generate_queen_qmoves(bitboard* b, movelist* l);
	static void generate_queen_cmoves(bitboard* b, movelist* l);

	template<bool side_to_move>
	static void generate_king_qmoves(bitboard* b, movelist* l)
	{
		if (!side_to_move) {
			if ((b->castling_rights & b->w_kingside) != 0 && (b->occupancy[2] & w_kingside_castling_mask) == 0ULL) {
				l->moves[l->size] = bit_move(4, 6, bit_move::kingside_castle, KING, EMPTY);
				l->size++;
			}
			if ((b->castling_rights & b->w_queenside) != 0 && (b->occupancy[2] & w_queenside_castling_mask) == 0ULL) {
				l->moves[l->size] = bit_move(4, 2, bit_move::queenside_castle, KING, EMPTY);
				l->size++;
			}
		}
		else {
			if ((b->castling_rights & b->b_kingside) != 0 && (b->occupancy[2] & b_kingside_castling_mask) == 0ULL) {
				l->moves[l->size] = bit_move(60, 62, bit_move::kingside_castle, KING, EMPTY);
				l->size++;
			}
			if ((b->castling_rights & b->b_queenside) != 0 && (b->occupancy[2] & b_queenside_castling_mask) == 0ULL) {
				l->moves[l->size] = bit_move(60, 58, bit_move::queenside_castle, KING, EMPTY);
				l->size++;
			}
		}
		uint64_t king = b->bbs[KING][side_to_move];
		uint8_t origin = BitScanForward64(king);
		uint8_t target;
		
		uint64_t king_attacks = attacks::king_attacks[origin] & (~b->occupancy[2]);

		while (king_attacks != 0ULL) {
			target = BitScanForward64(king_attacks);
			l->moves[l->size] = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, KING, EMPTY);
			l->size++;
			king_attacks &= king_attacks - 1;
		}
	}

	static void generate_king_cmoves(bitboard* b, movelist* l);

	static void generate_all_captures(bitboard* b, movelist* l);
	static void generate_all_quiet_moves(bitboard* b, movelist* l);

	static void generate_all_pseudo_legal_moves(bitboard* b, movelist* l);

};

