#pragma once

#include <list>
#include "bit_move.h"
#include "bitboard_util.h"
#include "bitboard.h"
#include "movelist.h"

class movegen
{
public:
	// default constructor
	movegen();
	// default destructor
	~movegen();

	// single square pawn push offset (absolute value)
	static const int PUSH_OFFSET = 8;
	// double square pawn push offset (absolute value)
	static const int DOUBLE_PUSH_OFFSET = 16;
	// white pawn push offset
	static const int NORTH = 8;
	// black pawn push offset
	static const int SOUTH = -8;

	/**
	 * Generate all quiet pawn moves.
	 *
	 * \tparam side_to_move boolean indicating side to move -- 0 = white; 1 = black -- used to reduce low level branching
	 *
	 * \param b pointer to board representation
	 * \param l pointer to movelist
	 */
	template<bool side_to_move>
	static void generate_pawn_qmoves(bitboard* b, movelist* l) {
		uint8_t target;
		// fetching the rank where pawns land after the first ever single-square pawn push 
		// -> 3rd rank for white or 6th rank for black
		uint64_t single_push_rank = (side_to_move) ? sixth_rank : third_rank;
		uint64_t last_rank = (side_to_move) ? first_rank : eighth_rank;

		uint64_t pushes = (side_to_move)
			? ((b->bbs[PAWN][side_to_move] >> PUSH_OFFSET) & ~b->occupancy[2])
			: ((b->bbs[PAWN][side_to_move] << PUSH_OFFSET) & ~b->occupancy[2]);
		uint64_t double_pushes = (side_to_move)
			? (((pushes & single_push_rank) >> PUSH_OFFSET) & ~b->occupancy[2])
			: (((pushes & single_push_rank) << PUSH_OFFSET) & ~b->occupancy[2]);

		pushes = pushes & ~last_rank;

		uint8_t push_offset = (side_to_move) ? SOUTH : NORTH;
		uint8_t double_push_offset = 2 * push_offset;


		while (pushes != 0ULL) {
			target = BitScanForward64(pushes);
			l->moves[l->size].m = bit_move((uint8_t)target - push_offset, (uint8_t)target, bit_move::quiet_move, PAWN, EMPTY);
			l->size++;
			pushes = reset_lsb(pushes);
		}
		while (double_pushes != 0ULL) {
			target = BitScanForward64(double_pushes);
			l->moves[l->size].m = bit_move((uint8_t)target - double_push_offset, (uint8_t)target, bit_move::double_pawn_push, PAWN, EMPTY);
			l->size++;
			double_pushes = reset_lsb(double_pushes);
		}
	}

	/**
	 * Generate all pawn captures, including promotion captures and non-capture promotions.
	 *
	 * Normal promotions are included, because promotions do not constitute quiet moves.
	 *
	 * \tparam side_to_move boolean indicating side to move -- 0 = white; 1 = black -- used to reduce low level branching
	 *
	 * \param b pointer to the board representation
	 * \param l pointer to the movelist
	 */
	template<bool side_to_move>
	static void generate_pawn_cmoves(bitboard* b, movelist* l)
	{
		uint8_t origin;
		uint8_t target;
		// getting pawn bitboard of side to move
		uint64_t pawns = b->bbs[PAWN][side_to_move];
		// generating a bitboard of push targets
		const uint64_t pushes = (side_to_move)
			? ((pawns >> PUSH_OFFSET) & ~b->occupancy[2])
			: ((pawns << PUSH_OFFSET) & ~b->occupancy[2]);
		// getting the rank of promotion -> 8th rank when white is to move, 1st rank when black is to move
		uint64_t promotion_rank = (side_to_move) ? first_rank : eighth_rank;
		// identifying which pushes promote a pawn
		uint64_t promotion_pushes = pushes & promotion_rank;
		// fetching the push direction (signed : white to move => 8 or black to move => -8)
		const int8_t offset = (side_to_move) ? SOUTH : NORTH;
		// zero-initializing the attack bitboard
		uint64_t pawn_attacks = 0ULL;
		// zero-initializing the type of the captured piece
		uint8_t capture_type = EMPTY;

		// iterating over all pawns of the side to move ...
		while (pawns != 0ULL) {
			// getting the next pawn to process
			origin = BitScanForward64(pawns);
			pawn_attacks = attacks::pawn_attacks[side_to_move][origin] & (b->occupancy[!side_to_move]);
			// if previous move was a double pawn push ...
			if (b->ep_target_square != 0ULL) {
				// if current pawn can capture en passant -> add target square to attacks
				pawn_attacks |= attacks::pawn_attacks[side_to_move][origin] & b->ep_target_square;
			}
			// as long as there are possible captures (or promotion captures) ...
			while (pawn_attacks != 0ULL) {
				// get next capture target square
				target = BitScanForward64(pawn_attacks);
				// retrieve type of captured piece
				capture_type = b->piece_type_from_index(target);
				// the target is the en passant target square => the current capture is an en passant capture
				if ((1ULL << target) == b->ep_target_square) {
					// add the move to the movelist
					l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::ep_capture, PAWN, PAWN);
					// increment movelist size counter
					l->size++;
				}
				// if the capture is a promotion capture ...
				else if ((1ULL << target) & promotion_rank) {
					// [the captured piece cannot be a pawn; pawn only exist between the 2nd and 7th rank]
					// add all moves (one for each piece type the pawn can promote to) to the movelist
					l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::queen_capture_promotion, PAWN, capture_type);
					l->moves[l->size + 1].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::rook_capture_promotion, PAWN, capture_type);
					l->moves[l->size + 2].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::bishop_capture_promotion, PAWN, capture_type);
					l->moves[l->size + 3].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::knight_capture_promotion, PAWN, capture_type);
					// increment movelist size counter
					l->size += 4;
				}
				// move is a "standard" capture
				else {
					// add move to movelist
					l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, PAWN, capture_type);
					// increment movelist size counter
					l->size++;
				}
				// reset the lsb of the pawn attacks bitboard
				pawn_attacks = reset_lsb(pawn_attacks);
			}
			// reset the lsb of the pawn bitboard
			pawns = reset_lsb(pawns);
		}
		// if there are promotion pushes ...
		while (promotion_pushes != 0ULL) {
			// fetch the first push target (all targets exist on the 1st or 8th rank)
			target = BitScanForward64(promotion_pushes);
			// add moves to movelist (one move for each piece type the pawn can promote to)
			l->moves[l->size].m = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::queen_promotion, PAWN, EMPTY);
			l->moves[l->size + 1].m = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::rook_promotion, PAWN, EMPTY);
			l->moves[l->size + 2].m = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::bishop_promotion, PAWN, EMPTY);
			l->moves[l->size + 3].m = bit_move((uint8_t)target - offset, (uint8_t)target, bit_move::knight_promotion, PAWN, EMPTY);
			// increment movelist size counter
			l->size += 4;
			// reset the lsb of the promotion pushes bitboard
			promotion_pushes = reset_lsb(promotion_pushes);
		}
	}

	/**
	 * @brief Generates all quiet knight moves. (no captures)
	 *
	 * @param b pointer to the board representation
	 * @param l pointer to the movelist
	 *
	 * @tparam side_to_move boolean indicating side to move
	 */
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
				l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, KNIGHT, EMPTY);
				l->size++;
				knight_attacks = reset_lsb(knight_attacks);
			}
			knights = reset_lsb(knights);
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
				l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, KNIGHT, capture_type);
				l->size++;
				knight_attacks = reset_lsb(knight_attacks);
			}
			knights = reset_lsb(knights);
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
				l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, BISHOP, EMPTY);
				l->size++;
				bishop_attacks = reset_lsb(bishop_attacks);
			}
			bishops = reset_lsb(bishops);
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
				l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, BISHOP, capture_type);
				l->size++;
				bishop_attacks = reset_lsb(bishop_attacks);
			}
			bishops = reset_lsb(bishops);
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
				l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, ROOK, EMPTY);
				l->size++;
				rook_attacks = reset_lsb(rook_attacks);
			}
			rooks = reset_lsb(rooks);
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
				l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::capture, ROOK, capture_type);
				l->size++;
				rook_attacks = reset_lsb(rook_attacks);
			}
			rooks &= reset_lsb(rooks);
		}
	}

	static void generate_queen_qmoves(bitboard* b, movelist* l);
	static void generate_queen_cmoves(bitboard* b, movelist* l);

	// TODO: Castling bugfix...
	template<bool side_to_move>
	static void generate_king_qmoves(bitboard* b, movelist* l)
	{
		// if white has the move
		if (!side_to_move) {
			if ((b->castling_rights & b->w_kingside) && !(b->occupancy[2] & w_kingside_castling_mask)) {
				l->moves[l->size].m = bit_move(E1, G1, bit_move::kingside_castle, KING, EMPTY);
				l->size++;
			}
			if ((b->castling_rights & b->w_queenside) && !(b->occupancy[2] & w_queenside_castling_mask)) {
				l->moves[l->size].m = bit_move(E1, C1, bit_move::queenside_castle, KING, EMPTY);
				l->size++;
			}
		}
		// if black has the move
		else {
			if ((b->castling_rights & b->b_kingside) && !(b->occupancy[2] & b_kingside_castling_mask)) {
				l->moves[l->size].m = bit_move(E8, G8, bit_move::kingside_castle, KING, EMPTY);
				l->size++;
			}
			if ((b->castling_rights & b->b_queenside) != 0 && !(b->occupancy[2] & b_queenside_castling_mask)) {
				l->moves[l->size].m = bit_move(E8, C8, bit_move::queenside_castle, KING, EMPTY);
				l->size++;
			}
		}
		uint8_t origin = b->king_positions[side_to_move];
		uint8_t target;

		uint64_t king_attacks = attacks::king_attacks[origin] & (~b->occupancy[2]);

		while (king_attacks != 0ULL) {
			target = BitScanForward64(king_attacks);
			l->moves[l->size].m = bit_move((uint8_t)origin, (uint8_t)target, bit_move::quiet_move, KING, EMPTY);
			l->size++;
			king_attacks = reset_lsb(king_attacks);
		}
	}

	static void generate_king_cmoves(bitboard* b, movelist* l);

	static void generate_all_captures(bitboard* b, movelist* l);
	static void generate_all_quiet_moves(bitboard* b, movelist* l);

	static void generate_all_pseudo_legal_moves(bitboard* b, movelist* l);

};

