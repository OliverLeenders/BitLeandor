#pragma once
#include <cstdint>
#include <iostream>
#include <intrin.h>
#include "constants.h"

[[nodiscard]] inline uint64_t set_bit(int bit) {
	return 1ULL << bit;
}


[[nodiscard]] inline bool is_bit_set(uint64_t bitboard, int bit) {
	return bitboard & set_bit(bit);
}

[[nodiscard]] inline uint64_t reset_lsb(uint64_t bb) {
	return bb & bb - 1;
}



inline void print_bitboard(uint64_t bitboard) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int bit = (7 - i) * 8 + j;
			if (is_bit_set(bitboard, bit)) {
				std::cout << "x ";
			}
			else {
				std::cout << ". ";
			}
		}
		std::cout << "\n";
	}
	std::cout << std::endl;
	std::cout << bitboard << std::endl;
	return;
}



// not A file constant
const uint64_t not_a_file = 18374403900871474942ULL;

// not H file constant
const uint64_t not_h_file = 9187201950435737471ULL;

// not HG file constant
const uint64_t not_hg_file = 4557430888798830399ULL;

// not AB file constant
const uint64_t not_ab_file = 18229723555195321596ULL;

const uint64_t first_rank = 255ULL;
const uint64_t second_rank = 65280ULL;
const uint64_t third_rank = 16711680ULL;
const uint64_t fourth_rank = 4278190080ULL;
const uint64_t fifth_rank = 1095216660480ULL;
const uint64_t sixth_rank = 280375465082880ULL;
const uint64_t seventh_rank = 71776119061217280ULL;
const uint64_t eighth_rank = 18374686479671623680ULL;

const uint64_t a_file = 72340172838076673ULL;
const uint64_t b_file = 144680345676153346ULL;
const uint64_t c_file = 289360691352306692ULL;
const uint64_t d_file = 1157442765409226768ULL;
const uint64_t e_file = 1157442765409226768ULL;
const uint64_t f_file = 2314885530818453536ULL;
const uint64_t g_file = 4629771061636907072ULL;
const uint64_t h_file = 9259542123273814144ULL;



const uint64_t file_masks[NUM_SQUARES] = {
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file,
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file,
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file,
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file,
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file,
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file,
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file,
	a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file
};

const uint64_t rank_masks[NUM_SQUARES] = {
	first_rank,	   first_rank,	  first_rank,	 first_rank,	first_rank,	   first_rank,	  first_rank,	 first_rank,
	second_rank,   second_rank,   second_rank,   second_rank,   second_rank,   second_rank,   second_rank,   second_rank,
	third_rank,    third_rank,    third_rank,    third_rank,    third_rank,    third_rank,    third_rank,    third_rank,
	fourth_rank,   fourth_rank,   fourth_rank,   fourth_rank,   fourth_rank,   fourth_rank,   fourth_rank,   fourth_rank,
	fifth_rank,    fifth_rank,    fifth_rank,    fifth_rank,    fifth_rank,    fifth_rank,    fifth_rank,    fifth_rank,
	sixth_rank,    sixth_rank,    sixth_rank,    sixth_rank,    sixth_rank,    sixth_rank,    sixth_rank,    sixth_rank,
	seventh_rank,  seventh_rank,  seventh_rank,  seventh_rank,  seventh_rank,  seventh_rank,  seventh_rank,  seventh_rank,
	eighth_rank,   eighth_rank,   eighth_rank,   eighth_rank,   eighth_rank,   eighth_rank,   eighth_rank,   eighth_rank
};



const uint64_t files[8] = {
   a_file, b_file, c_file, d_file, e_file, f_file, g_file, h_file
};

const uint64_t w_queenside_castling_mask = 14ULL;
const uint64_t w_kingside_castling_mask = 96ULL;
const uint64_t b_queenside_castling_mask = 1008806316530991104ULL;
const uint64_t b_kingside_castling_mask = 6917529027641081856ULL;

/**
Getting the index of the least significant set bit
*/
[[nodiscard]] inline uint8_t BitScanForward64(uint64_t bb) {
	unsigned long index;
	_BitScanForward64(&index, bb);
	return (uint8_t)index;
}

[[nodiscard]] inline uint8_t BitScanReverse64(uint64_t bb) {
	unsigned long index;
	_BitScanReverse64(&index, bb);
	return (uint8_t)index;
}

[[nodiscard]] inline uint8_t PopCount64(uint64_t bb) {
	return (uint8_t)__popcnt64(bb);
}

[[nodiscard]] inline uint8_t PopCount8(uint8_t f) {
	return (uint8_t)__popcnt(f);
}
