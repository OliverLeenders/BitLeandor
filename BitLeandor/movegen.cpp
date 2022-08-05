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
void movegen::generate_pawn_qmoves(bitboard* b, movelist* l) {
	unsigned long target;
	int double_push_offset = 16;
	int push_offset = 8;
	
	uint64_t single_push_rank = (b->side_to_move) ? bitboard_util::sixth_rank : bitboard_util::third_rank;
	uint64_t last_rank = (b->side_to_move) ? bitboard_util::eighth_rank : bitboard_util::first_rank;
	
	uint64_t pushes = (b->side_to_move) ? ((b->pawns[b->side_to_move] >> push_offset) & ~b->pieces[2]) : ((b->pawns[b->side_to_move] << push_offset) & ~b->pieces[2]);
	uint64_t double_pushes = (b->side_to_move) ? (((pushes & single_push_rank) >> push_offset) & ~b->pieces[2]) : (((pushes & single_push_rank) << push_offset) & ~b->pieces[2]);
	uint64_t promotion_pushes = pushes & last_rank;
	
	pushes = pushes & ~last_rank;

	push_offset = (b->side_to_move) ? -8 : 8;
	double_push_offset = (b->side_to_move) ? -16 : 16;
	
	
	while (pushes != 0ULL) {
		_BitScanForward64(&target, pushes);
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::quiet_move, bitboard_util::pawn, bitboard_util::empty));
		l->size++;
		pushes ^= 1ULL << target;
	}
	while (double_pushes != 0ULL) {
		_BitScanForward64(&target, double_pushes);
		l->moves[l->size] = *(new bit_move(target - double_push_offset, target, bit_move::double_pawn_push, bitboard_util::pawn, bitboard_util::empty));
		l->size++;
		double_pushes ^= 1ULL << target;
	}
	while (promotion_pushes != 0ULL) {
		_BitScanForward64(&target, promotion_pushes);
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::queen_promotion, bitboard_util::pawn, bitboard_util::empty));
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::rook_promotion, bitboard_util::pawn, bitboard_util::empty));
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::bishop_promotion, bitboard_util::pawn, bitboard_util::empty));
		l->moves[l->size] = *(new bit_move(target - push_offset, target, bit_move::knight_promotion, bitboard_util::pawn, bitboard_util::empty));
		l->size += 4;
		promotion_pushes ^= 1ULL << target;
	}
}
/// <summary>
/// Generate pawn captures including en-passant captures and capture promotions
/// </summary>
/// <param name="b">the board</param>
/// <param name="l">the movelist to append to</param>
void movegen::generate_pawn_cmoves(bitboard* b, movelist* l)
{
	unsigned long origin;
	unsigned long target;
	uint64_t pawns = b->pawns[b->side_to_move];
	while (pawns != 0ULL) {
		_BitScanForward64(&origin, pawns);
		uint64_t promotion_rank = (b->side_to_move) ? bitboard_util::first_rank : bitboard_util::eighth_rank;
		uint64_t pawn_attacks = attacks::pawn_attacks[b->side_to_move][origin] & (b->pieces[!b->side_to_move] & (1ULL << b->ep_target_square));
		while (pawn_attacks != 0ULL) {
			_BitScanForward64(&target, pawn_attacks);
			uint8_t capture_type = b->piece_type_from_index(target);
			if (target == b->ep_target_square) {
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::ep_capture, bitboard_util::pawn, capture_type));
				l->size++;
			}
			else if ((1ULL << target) & promotion_rank) {
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::queen_capture_promotion, bitboard_util::pawn, capture_type));
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::rook_capture_promotion, bitboard_util::pawn, capture_type));
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::bishop_capture_promotion, bitboard_util::pawn, capture_type));
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::knight_capture_promotion, bitboard_util::pawn, capture_type));
				l->size += 4;
			}
			else {
				l->moves[l->size] = *(new bit_move(origin, target, bit_move::capture, bitboard_util::pawn, capture_type));
				l->size++;
			}
			pawn_attacks ^= 1ULL << target;
		}
		pawns ^= 1ULL << origin;
	}
}

void movegen::generate_knight_qmoves(bitboard* b, movelist* l)
{
	uint64_t knights = b->knights[b->side_to_move];
	uint64_t pieces = b->pieces[2];
	unsigned long origin;
	unsigned long target;
	while (knights != 0ULL) {
		_BitScanForward64(&origin, knights);
		uint64_t knight_attacks = attacks::knight_attacks[origin] & (~pieces);
		while (knight_attacks != 0ULL) {
			_BitScanForward64(&target, knight_attacks);
			l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move, bitboard_util::knight, bitboard_util::empty));
			l->size++;
			knight_attacks ^= 1ULL << target;
		}
		knights ^= 1ULL << origin;
	}
}

void movegen::generate_knight_cmoves(bitboard* b, movelist* l)
{
	uint64_t knights = b->knights[b->side_to_move];
	uint64_t opp_pieces = b->pieces[!b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (knights != 0ULL) {
		_BitScanForward64(&origin, knights);
		uint64_t knight_attacks = attacks::knight_attacks[origin] & opp_pieces;
		while (knight_attacks != 0ULL) {
			_BitScanForward64(&target, knight_attacks);
			uint8_t capture_type = b->piece_type_from_index(target);
			l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move, bitboard_util::knight, capture_type));
			l->size++;
			knight_attacks ^= 1ULL << target;
		}
		knights ^= 1ULL << origin;
	}
}

void movegen::generate_bishop_qmoves(bitboard* b, movelist* l)
{
	uint64_t bishops = b->bishops[b->side_to_move] | b->queens[b->side_to_move];
	unsigned long origin;
	unsigned long target; 
	while (bishops != 0ULL)
	{
		_BitScanForward64(&origin, bishops);
		uint64_t bishop_attacks = attacks::get_bishop_attacks(origin, b->pieces[2]) & (~b->pieces[2]);
		while (bishop_attacks != 0ULL) {
			_BitScanForward64(&target, bishop_attacks);
			l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move, bitboard_util::bishop, bitboard_util::empty));
			l->size++;
			bishop_attacks ^= 1ULL << target;
		}
		bishops ^= 1ULL << origin;
	}
}

void movegen::generate_bishop_cmoves(bitboard* b, movelist* l)
{
	uint64_t bishops = b->bishops[b->side_to_move] | b->queens[b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (bishops != 0ULL)
	{
		_BitScanForward64(&origin, bishops);
		uint64_t bishop_attacks = attacks::get_bishop_attacks(origin, b->pieces[2]) & b->pieces[!b->side_to_move];
		while (bishop_attacks != 0ULL) {
			_BitScanForward64(&target, bishop_attacks);
			uint8_t capture_type = b->piece_type_from_index(target);
			l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move, bitboard_util::bishop, capture_type));
			l->size++;
			bishop_attacks ^= 1ULL << target;
		}
		bishops ^= 1ULL << origin;
	}
}

void movegen::generate_rook_qmoves(bitboard* b, movelist* l)
{
	uint64_t rooks = b->rooks[b->side_to_move] | b->queens[b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (rooks != 0ULL)
	{
		_BitScanForward64(&origin, rooks);
		uint64_t rook_attacks = attacks::get_rook_attacks(origin, b->pieces[2]) & (~b->pieces[2]);
		while (rook_attacks != 0ULL) {
			_BitScanForward64(&target, rook_attacks);
			l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move, bitboard_util::rook, bitboard_util::empty));
			l->size++;
			rook_attacks ^= 1ULL << target;
		}
		rooks ^= 1ULL << origin;
	}
}

void movegen::generate_rook_cmoves(bitboard* b, movelist* l)
{
	uint64_t rooks = b->rooks[b->side_to_move] | b->queens[b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (rooks != 0ULL)
	{
		_BitScanForward64(&origin, rooks);
		uint64_t rook_attacks = attacks::get_rook_attacks(origin, b->pieces[2]) & (b->pieces[!b->side_to_move]);
		while (rook_attacks != 0ULL) {
			_BitScanForward64(&target, rook_attacks);
			uint8_t capture_type = b->piece_type_from_index(target);
			l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move, bitboard_util::rook, capture_type));
			l->size++;
			rook_attacks ^= 1ULL << target;
		}
		rooks ^= 1ULL << origin;
	}
}

void movegen::generate_king_qmoves(bitboard* b, movelist* l)
{
	if (!b->side_to_move) {
		if (b->castling_rights == b->w_kingside && (b->pieces[2] & bitboard_util::w_kingside_castling_mask) == 0ULL) {
			l->moves[l->size] = *(new bit_move(4, 6, bit_move::kingside_castle, bitboard_util::king, bitboard_util::empty));
			l->size++;
		}
		if (b->castling_rights == b->w_queenside && (b->pieces[2] & bitboard_util::w_queenside_castling_mask) == 0ULL) {
			l->moves[l->size] = *(new bit_move(4, 2, bit_move::queenside_castle, bitboard_util::king, bitboard_util::empty));
			l->size++;
		}
	}
	else {
		if (b->castling_rights == b->b_kingside && (b->pieces[2] & bitboard_util::b_kingside_castling_mask) == 0ULL) {
			l->moves[l->size] = *(new bit_move(60, 62, bit_move::kingside_castle, bitboard_util::king, bitboard_util::empty));
			l->size++;
		}
		if (b->castling_rights == b->b_queenside && (b->pieces[2] & bitboard_util::b_queenside_castling_mask) == 0ULL) {
			l->moves[l->size] = *(new bit_move(60, 58, bit_move::queenside_castle, bitboard_util::king, bitboard_util::empty));
			l->size++;
		}
	}
	uint64_t king = b->kings[b->side_to_move];
	unsigned long origin;
	unsigned long target;
	_BitScanForward64(&origin, king);
	uint64_t king_attacks = attacks::king_attacks[origin] & (~b->pieces[2]);

	while (king_attacks != 0ULL) {
		_BitScanForward64(&target, king_attacks);
		l->moves[l->size] = *(new bit_move(origin, target, bit_move::quiet_move, bitboard_util::king, bitboard_util::empty));
		l->size++;
		king_attacks ^= 1ULL << target;
	}
}

void movegen::generate_king_cmoves(bitboard* b, movelist* l)
{
	uint64_t king = b->kings[b->side_to_move];
	uint64_t opp_pieces = b->pieces[!b->side_to_move];
	unsigned long origin;
	unsigned long target;
	_BitScanForward64(&origin, king);
	uint64_t king_attacks = attacks::king_attacks[origin] & opp_pieces;
	while (king_attacks != 0ULL) {
		_BitScanForward64(&target, king_attacks);
		uint8_t capture_type = b->piece_type_from_index(target);
		l->moves[l->size] = *(new bit_move(origin, target, bit_move::capture, bitboard_util::king, capture_type));
		l->size++;
		king_attacks ^= 1ULL << target;
	}
}

void movegen::generate_all_captures(bitboard* b, movelist* l)
{
	generate_pawn_cmoves(b, l);
	generate_knight_cmoves(b, l);
	generate_bishop_cmoves(b, l);
	generate_rook_cmoves(b, l);
	generate_king_cmoves(b, l);
}

void movegen::generate_all_quiet_moves(bitboard* b, movelist* l)
{
	generate_pawn_qmoves(b, l);
	generate_knight_qmoves(b, l);
	generate_bishop_qmoves(b, l);
	generate_rook_qmoves(b, l);
	generate_king_qmoves(b, l);
}

void movegen::generate_all_pseudo_legal_moves(bitboard* b, movelist* l)
{
	generate_all_captures(b, l);
	generate_all_quiet_moves(b, l);
}




