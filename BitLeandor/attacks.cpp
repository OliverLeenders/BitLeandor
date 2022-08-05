#include "attacks.h"

uint64_t attacks::king_attacks[64] = { 0ULL };
uint64_t attacks::knight_attacks[64] = { 0ULL };
uint64_t attacks::pawn_attacks[2][64] = { 0ULL };
uint64_t attacks::rook_attack_masks[64] = { 0ULL };
uint64_t attacks::bishop_attack_masks[64] = { 0ULL };
uint64_t attacks::rook_magics[64] = { 0ULL };
uint64_t attacks::bishop_magics[64] = { 0ULL };
uint64_t attacks::rook_attacks[64][4096] = { 0ULL };
uint64_t attacks::bishop_attacks[64][512] = { 0ULL };

void attacks::init_attack_tables() {
	init_king_attacks();
	init_knight_attacks();
	init_pawn_attacks();
	init_rook_attack_masks();
	init_bishop_attack_masks();
	return;
}

void attacks::init_king_attacks() {
	for (int i = 0; i < 64; i++) {
		uint64_t pattern = 0ULL;
		if (i < 56) {
			pattern |= bitboard_util::set_bit(i + 8);
		}
		if (i >= 8) {
			pattern |= bitboard_util::set_bit(i - 8);
		}
		if (i % 8 < 7) {
			pattern |= bitboard_util::set_bit(i + 1);
		}
		if (i % 8 > 0) {
			pattern |= bitboard_util::set_bit(i - 1);
		}
		if (i < 56 && i % 8 < 7) {
			pattern |= bitboard_util::set_bit(i + 9);
		}
		if (i < 56 && i % 8 > 0) {
			pattern |= bitboard_util::set_bit(i + 7);
		}
		if (i >= 8 && i % 8 < 7) {
			pattern |= bitboard_util::set_bit(i - 7);
		}
		if (i >= 8 && i % 8 > 0) {
			pattern |= bitboard_util::set_bit(i - 9);
		}
		king_attacks[i] = pattern;
	}
	return;
}

void attacks::init_knight_attacks() {
	for (int i = 0; i < 64; i++) {
		uint64_t pattern = 0ULL;
		// +  6 vector
		if (i % 8 > 1 && i < 56) {
			pattern |= bitboard_util::set_bit(i + 6);
		}
		// + 15 vector 
		if (i % 8 > 0 && i < 48) {
			pattern |= bitboard_util::set_bit(i + 15);
		}
		// + 17 vector
		if (i % 8 < 7 && i < 48) {
			pattern |= bitboard_util::set_bit(i + 17);
		}
		// + 10 vector
		if (i % 8 < 6 && i < 56) {
			pattern |= bitboard_util::set_bit(i + 10);
		}
		// -  6 vector
		if (i % 8 < 6 && i >= 8) {
			pattern |= bitboard_util::set_bit(i - 6);
		}
		// - 15 vector
		if (i % 8 < 7 && i >= 16) {
			pattern |= bitboard_util::set_bit(i - 15);
		}
		// - 17 vector
		if (i % 8 > 0 && i >= 16) {
			pattern |= bitboard_util::set_bit(i - 17);
		}
		// - 10 vector
		if (i % 8 > 1 && i >= 8) {
			pattern |= bitboard_util::set_bit(i - 10);
		}
		// set table value
		knight_attacks[i] = pattern;
	}
	return;
}
void attacks::init_pawn_attacks() {
	for (int i = 8; i < 56; i++) {
		uint64_t pattern_w = 0ULL;
		uint64_t pattern_b = 0ULL;
		if (i % 8 != 0) {
			pattern_w |= bitboard_util::set_bit(i + 7);
			pattern_b |= bitboard_util::set_bit(i - 9);
		}
		if (i % 8 != 7) {
			pattern_w |= bitboard_util::set_bit(i + 9);
			pattern_b |= bitboard_util::set_bit(i - 7);
		}
		pawn_attacks[0][i] = pattern_w;
		pawn_attacks[1][i] = pattern_b;
	}
	return;
}

void attacks::init_bishop_attack_masks() {
	for (int i = 0; i < 64; i++) {
		uint64_t pattern = 0ULL;
		// + 7 vector
		for (int j = i + 7; j % 8 != 7 && j < 56; j += 7) {
			pattern |= bitboard_util::set_bit(j);
		}
		// + 9 vector
		for (int j = i + 9; j % 8 != 0 && j < 56; j += 9) {
			pattern |= bitboard_util::set_bit(j);
		}
		// - 7 vector
		for (int j = i - 7; j % 8 != 0 && j >= 8; j -= 7) {
			pattern |= bitboard_util::set_bit(j);
		}
		// - 9 vector
		for (int j = i - 9; j % 8 != 7 && j >= 8; j -= 9) {
			pattern |= bitboard_util::set_bit(j);
		}
		// store pattern
		bishop_attack_masks[i] = pattern;
	}
	return;
}

// credit goes to maksim korzh for this code
// my own approach introduced a bug so i borrowed his
void attacks::init_rook_attack_masks() {
	for (int i = 0; i < 64; i++) {
		// result attacks bitboard
		uint64_t attacks = 0ULL;

		// init ranks & files
		int r, f;

		// init target rank & files
		int tr = i / 8;
		int tf = i % 8;

		// mask relevant rook occupancy bits
		for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
		for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
		for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
		for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));
		
		rook_attack_masks[i] = attacks;
	}
	//std::cout << "over" << std::endl;
	return;
}

uint64_t attacks::compute_rook_attacks(int square, uint64_t blockers) {
	uint64_t pattern = 0ULL;
	// + 8 vector
	for (int j = square + 8; j < 64; j += 8) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// + 1 vector
	for (int j = square + 1; j % 8 != 0; j++) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// - 8 vector
	for (int j = square - 8; j >= 0; j -= 8) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// - 1 vector
	for (int j = square - 1; j % 8 != 7 && j >= 0; j--) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// return pattern
	return pattern;
}

uint64_t attacks::compute_bishop_attacks(int square, uint64_t blockers) {
	uint64_t pattern = 0ULL;
	// + 7 vector
	for (int j = square + 7; j % 8 != 7 && j < 64; j += 7) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// + 9 vector
	for (int j = square + 9; j % 8 != 0 && j < 64; j += 9) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// - 7 vector
	for (int j = square - 7; j % 8 != 0 && j >= 0; j -= 7) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// - 9 vector
	for (int j = square - 9; j % 8 != 7 && j >= 0; j -= 9) {
		pattern |= bitboard_util::set_bit(j);
		if (blockers & bitboard_util::set_bit(j)) break;
	}
	// return pattern
	return pattern;
}

const int attacks::num_relevant_bits_bishop[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

const int attacks::num_relevant_bits_rook[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};
