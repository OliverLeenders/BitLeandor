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



void movegen::generate_knight_qmoves(bitboard* b, movelist* l)
{
	uint64_t knights = b->bbs[KNIGHT][b->side_to_move];
	uint64_t pieces = b->occupancy[2];
	unsigned long origin;
	unsigned long target;
	while (_BitScanForward64(&origin, knights)) {
		uint64_t knight_attacks = attacks::knight_attacks[origin] & (~pieces);
		while (knight_attacks != 0ULL) {
			_BitScanForward64(&target, knight_attacks);
			l->moves[l->size] = bit_move(origin, target, bit_move::quiet_move, KNIGHT, EMPTY);
			l->size++;
			knight_attacks ^= 1ULL << target;
		}
		knights ^= 1ULL << origin;
	}
}

void movegen::generate_knight_cmoves(bitboard* b, movelist* l)
{
	uint64_t knights = b->bbs[KNIGHT][b->side_to_move];
	uint64_t opp_pieces = b->occupancy[!b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (knights != 0ULL) {
		_BitScanForward64(&origin, knights);
		uint64_t knight_attacks = attacks::knight_attacks[origin] & opp_pieces;
		while (_BitScanForward64(&target, knight_attacks)) {
			uint8_t capture_type = b->piece_type_from_index(target);
			l->moves[l->size] = bit_move(origin, target, bit_move::capture, KNIGHT, capture_type);
			l->size++;
			knight_attacks ^= 1ULL << target;
		}
		knights ^= 1ULL << origin;
	}
}

void movegen::generate_bishop_qmoves(bitboard* b, movelist* l)
{
	uint64_t bishops = b->bbs[BISHOP][b->side_to_move];
	unsigned long origin;
	unsigned long target; 
	while (bishops != 0ULL)
	{
		_BitScanForward64(&origin, bishops);
		uint64_t bishop_attacks = attacks::get_bishop_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
		while (_BitScanForward64(&target, bishop_attacks)) {
			l->moves[l->size] = bit_move(origin, target, bit_move::quiet_move, BISHOP, EMPTY);
			l->size++;
			bishop_attacks ^= 1ULL << target;
		}
		bishops ^= 1ULL << origin;
	}
}

void movegen::generate_bishop_cmoves(bitboard* b, movelist* l)
{
	uint64_t bishops = b->bbs[BISHOP][b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (bishops != 0ULL)
	{
		_BitScanForward64(&origin, bishops);
		uint64_t bishop_attacks = attacks::get_bishop_attacks(origin, b->occupancy[2]) & b->occupancy[!b->side_to_move];
		while (_BitScanForward64(&target, bishop_attacks)) {
			uint8_t capture_type = b->piece_type_from_index(target);
			l->moves[l->size] = bit_move(origin, target, bit_move::capture, BISHOP, capture_type);
			l->size++;
			bishop_attacks ^= 1ULL << target;
		}
		bishops ^= 1ULL << origin;
	}
}

void movegen::generate_rook_qmoves(bitboard* b, movelist* l)
{
	uint64_t rooks = b->bbs[ROOK][b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (rooks != 0ULL)
	{
		_BitScanForward64(&origin, rooks);
		uint64_t rook_attacks = attacks::get_rook_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
		while (_BitScanForward64(&target, rook_attacks)) {
			l->moves[l->size] = bit_move(origin, target, bit_move::quiet_move, ROOK, EMPTY);
			l->size++;
			rook_attacks ^= 1ULL << target;
		}
		rooks ^= 1ULL << origin;
	}
}

void movegen::generate_rook_cmoves(bitboard* b, movelist* l)
{
	uint64_t rooks = b->bbs[ROOK][b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (rooks != 0ULL)
	{
		_BitScanForward64(&origin, rooks);
		uint64_t rook_attacks = attacks::get_rook_attacks(origin, b->occupancy[2]) & (b->occupancy[!b->side_to_move]);
		
		while (_BitScanForward64(&target, rook_attacks)) {
			uint8_t capture_type = b->piece_type_from_index(target);
			l->moves[l->size] = bit_move(origin, target, bit_move::capture, ROOK, capture_type);
			l->size++;
			rook_attacks ^= 1ULL << target;
		}
		rooks ^= 1ULL << origin;
	}
}

void movegen::generate_queen_qmoves(bitboard* b, movelist* l)
{
	uint64_t queens = b->bbs[QUEEN][b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (queens != 0ULL)
	{
		_BitScanForward64(&origin, queens);
		uint64_t queen_attacks = attacks::get_rook_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
		queen_attacks |= attacks::get_bishop_attacks(origin, b->occupancy[2]) & (~b->occupancy[2]);
		while (_BitScanForward64(&target, queen_attacks)) {
			l->moves[l->size] = bit_move(origin, target, bit_move::quiet_move, QUEEN, EMPTY);
			l->size++;
			queen_attacks ^= 1ULL << target;
		}
		queens ^= 1ULL << origin;
	}
}

void movegen::generate_queen_cmoves(bitboard* b, movelist* l)
{
	uint64_t queens = b->bbs[QUEEN][b->side_to_move];
	unsigned long origin;
	unsigned long target;
	while (queens != 0ULL)
	{
		_BitScanForward64(&origin, queens);
		uint64_t queen_attacks = attacks::get_rook_attacks(origin, b->occupancy[2]) & (b->occupancy[!b->side_to_move]);
		queen_attacks |= attacks::get_bishop_attacks(origin, b->occupancy[2]) & (b->occupancy[!b->side_to_move]);
		while (_BitScanForward64(&target, queen_attacks)) {
			uint8_t capture_type = b->piece_type_from_index(target);
			l->moves[l->size] = bit_move(origin, target, bit_move::capture, QUEEN, capture_type);
			l->size++;
			queen_attacks ^= 1ULL << target;
		}
		queens ^= 1ULL << origin;
	}
}


void movegen::generate_king_cmoves(bitboard* b, movelist* l)
{
	uint64_t king = b->bbs[KING][b->side_to_move];
	uint64_t opp_pieces = b->occupancy[!b->side_to_move];
	unsigned long origin;
	unsigned long target;
	_BitScanForward64(&origin, king);
	uint64_t king_attacks = attacks::king_attacks[origin] & opp_pieces;
	while (_BitScanForward64(&target, king_attacks)) {
		uint8_t capture_type = b->piece_type_from_index(target);
		l->moves[l->size] = bit_move(origin, target, bit_move::capture, KING, capture_type);
		l->size++;
		king_attacks ^= 1ULL << target;
	}
}

void movegen::generate_all_captures(bitboard* b, movelist* l)
{
	if (b->side_to_move) {
		generate_pawn_cmoves<true>(b, l);
	}
	else {
		generate_pawn_cmoves<false>(b, l);
	}
	generate_knight_cmoves(b, l);
	generate_bishop_cmoves(b, l);
	generate_rook_cmoves(b, l);
	generate_queen_cmoves(b, l);
	generate_king_cmoves(b, l);
}

void movegen::generate_all_quiet_moves(bitboard* b, movelist* l)
{
	if (b->side_to_move) {
		generate_pawn_qmoves<true>(b, l);
		generate_king_qmoves<true>(b, l);
	}
	else {
		generate_pawn_qmoves<false>(b, l);
		generate_king_qmoves<false>(b, l);
	}
	generate_knight_qmoves(b, l);
	generate_bishop_qmoves(b, l);
	generate_rook_qmoves(b, l);
	generate_queen_qmoves(b, l);
	
}

void movegen::generate_all_pseudo_legal_moves(bitboard* b, movelist* l)
{
	generate_all_captures(b, l);
	generate_all_quiet_moves(b, l);
}




