#include "movegen.h"

movegen::movegen()
{
}

movegen::~movegen()
{
}

/// <summary>
/// Generates all quiet pawn moves 
/// </summary>
/// <param name="b">the board</param>
/// <param name="l">the movelist to append to</param>
void movegen::generate_pawn_qmoves(bitboard b, movelist* l) {
	unsigned long target;
	int double_push_offset = (b.side_to_move) ? -16 : 16;
	int push_offset = (b.side_to_move) ? -8 : 8;
	uint64_t single_push_rank = (b.side_to_move) ? bitboard_util::sixth_rank : bitboard_util::third_rank;
	uint64_t last_rank = (b.side_to_move) ? bitboard_util::eighth_rank : bitboard_util::first_rank;
	uint64_t pushes = (b.pawns[0] << push_offset) & ~b.pieces[2];
	uint64_t double_pushes = ((pushes & single_push_rank) << push_offset) & ~b.pieces[2];
	uint64_t promotion_pushes = pushes & last_rank;
	pushes = pushes & ~last_rank;

	while (pushes != 0ULL) {
		_BitScanForward64(&target, pushes);
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::quiet_move));
		l->size++;
		pushes ^= 1ULL << target;
	}
	while (double_pushes != 0ULL) {
		_BitScanForward64(&target, double_pushes);
		l->moves[l->size] = *(new bit_move(target - double_push_offset, target, bit_move::double_pawn_push));
		l->size++;
		double_pushes ^= 1ULL << target;
	}
	while (promotion_pushes != 0ULL) {
		_BitScanForward64(&target, promotion_pushes);
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::queen_promotion));
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::rook_promotion));
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::bishop_promotion));
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::knight_promotion));
		l->size += 4;
		promotion_pushes ^= 1ULL << target;
	}
}
/// <summary>
/// Generate pawn captures including en-passant captures and capture promotions
/// </summary>
/// <param name="b">the board</param>
/// <param name="l">the movelist to append to</param>
void movegen::generate_pawn_cmoves(bitboard b, movelist* l)
{
	unsigned long origin;
	unsigned long target;
	uint64_t pawns = b.pawns[b.side_to_move];
	while (pawns != 0ULL) {
		_BitScanForward64(&origin, pawns);
		uint64_t promotion_rank = (b.side_to_move) ? bitboard_util::first_rank : bitboard_util::eighth_rank;
		uint64_t pawn_attacks = attacks::pawn_attacks[b.side_to_move][origin] & (b.pieces[!b.side_to_move] & (1ULL << b.ep_target_square));
		while (pawn_attacks != 0ULL) {
			_BitScanForward64(&target, pawn_attacks);
			if (target == b.ep_target_square) {
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::ep_capture));
				l->size++;
			}
			else if ((1ULL << target) & promotion_rank) {
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::queen_capture_promotion));
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::rook_capture_promotion));
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::bishop_capture_promotion));
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::knight_capture_promotion));
				l->size += 4;
			}
			else {
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::capture));
				l->size++;
			}
			pawn_attacks ^= 1ULL << target;
		}
		pawns ^= 1ULL << origin;
	}
}

void movegen::generate_knight_qmoves(bitboard b, movelist* l)
{
	uint64_t knights = b.knights[b.side_to_move];
	uint64_t pieces = b.pieces[2];
	unsigned long origin;
	unsigned long target;
	while (knights != 0ULL) {
		_BitScanForward64(&origin, knights);
		uint64_t knight_attacks = attacks::knight_attacks[origin] & (~pieces);
		while (knight_attacks != 0ULL) {
			_BitScanForward64(&target, knight_attacks);
			l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move));
			l->size++;
			knight_attacks ^= 1ULL << target;
		}
		knights ^= 1ULL << origin;
	}
}

void movegen::generate_knight_cmoves(bitboard b, movelist* l)
{
}


