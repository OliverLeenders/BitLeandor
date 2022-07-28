#include "bitboard_util.h"

bitboard_util::bitboard_util()
{
}

bitboard_util::~bitboard_util()
{
}

uint64_t bitboard_util::set_bit(int bit) {
	return 1ULL << bit;
}

bool bitboard_util::is_bit_set(uint64_t bitboard, int bit) {
	return bitboard & set_bit(bit);
}





int bitboard_util::index_64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};


void bitboard_util::print_bitboard(uint64_t bitboard) {
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