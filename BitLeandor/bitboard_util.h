#pragma once
#include <cstdint>
#include <iostream>
#include <intrin.h>

#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#pragma intrinsic(__popcnt64)

class bitboard_util
{
public:
	bitboard_util();
	static uint64_t set_bit(int bit);
	static void print_bitboard(uint64_t bitboard);
	static bool is_bit_set(uint64_t bitboard, int bit);
	~bitboard_util();


	// not A file constant
	static const uint64_t not_a_file = 18374403900871474942ULL;

	// not H file constant
	static const uint64_t not_h_file = 9187201950435737471ULL;

	// not HG file constant
	static const uint64_t not_hg_file = 4557430888798830399ULL;

	// not AB file constant
	static const uint64_t not_ab_file = 18229723555195321596ULL;

	static const uint64_t first_rank = 255ULL;
	static const uint64_t second_rank = 65280ULL;
	static const uint64_t third_rank = 16711680ULL;
	static const uint64_t sixth_rank = 280375465082880ULL;
	static const uint64_t seventh_rank = 71776119061217280ULL;
	static const uint64_t eighth_rank = 18374686479671623680ULL;
private:
	static int index_64[64];
};

