#include "utility.h"

void utility::remove_first_occurance(std::list<int>* l, int i) {
	std::list<int>::iterator itr;
	bool not_removed = true;
	for (itr = l->begin(); itr != l->end() && not_removed; ) {
		if (*itr == i) {
			itr = l->erase(itr);
			not_removed = true;
		}
		else {
			itr++;
		}
	}
}


uint64_t utility::random_64_bit_num()
{
	uint64_t num = 0;

	for (int i = 0; i < 64; i++)
	{
		num = num * 2 + rand() % 2;
	}
	//std::cout << num << std::endl;
	return num;
}

unsigned int utility::random_state = 1804289383;

unsigned int utility::pseudo_rand_32_bit_num()
{
	// get state
	int num = random_state;

	// XOR-shift 32 algorithm
	num ^= num << 13;
	num ^= num >> 17;
	num ^= num << 5;

	// update state
	random_state = num;
	// return generated number
	return num;
}

uint64_t utility::pseudo_rand_64_bit_num() {
	uint64_t n_1, n_2, n_3, n_4;
	n_1 = (uint64_t)(pseudo_rand_32_bit_num()) & 0xFFFFULL;
	n_2 = (uint64_t)(pseudo_rand_32_bit_num()) & 0xFFFFULL;
	n_3 = (uint64_t)(pseudo_rand_32_bit_num()) & 0xFFFFULL;
	n_4 = (uint64_t)(pseudo_rand_32_bit_num()) & 0xFFFFULL;
	return n_1 | (n_2 << 16) | (n_3 << 32) | (n_4 << 48);
}

uint64_t utility::magic_num_candidate() {
	return pseudo_rand_64_bit_num() & pseudo_rand_64_bit_num() & pseudo_rand_64_bit_num();
}

uint64_t utility::generate_occupancy_by_index(int index, int bit_count, uint64_t attack_mask) {
	uint64_t occupancy = 0ULL;

	for (int count = 0; count < bit_count; count++) {
		uint8_t square = BitScanForward64(attack_mask);
		if (is_bit_set(attack_mask, square))
		{
			attack_mask ^= set_bit(square);
		}
		else {
			attack_mask ^= 0ULL;
		}
		if (index & (1 << count)) {
			occupancy |= 1ULL << square;
		}
	}
	return occupancy;
}

uint64_t utility::generate_magic_number(int square, int relevant_bits, bool is_bishop) {
	uint64_t occupancies[4096] = { 0ULL };
	uint64_t attacks[4096] = { 0ULL };
	uint64_t used_attacks[4096] = { 0ULL };

	uint64_t attack_mask = is_bishop ? attacks::bishop_attack_masks[square] : attacks::rook_attack_masks[square];

	int occupancy_indices = 1ULL << relevant_bits;

	for (int i = 0; i < occupancy_indices; i++) {
		occupancies[i] = generate_occupancy_by_index(i, relevant_bits, attack_mask);
		attacks[i] = is_bishop ? attacks::compute_bishop_attacks(square, occupancies[i]) :
			attacks::compute_rook_attacks(square, occupancies[i]);
	}

	// test candidate magic numbers
	for (int i = 0; i < 100000000; i++) {
		uint64_t candidate = magic_num_candidate();
		if (PopCount64((attack_mask * candidate) & 0xFF00000000000000ULL) < 6) {
			continue;
		}
		for (int j = 0; j < 4096; j++) {
			used_attacks[j] = 0ULL;
		}
		int index;
		bool fail = false;
		for (index = 0; index < occupancy_indices && !fail; index++) {
			// init magic index
			int magic_index = (int)((occupancies[index] * candidate) >> (64 - relevant_bits));
			if (used_attacks[magic_index] == 0ULL) {
				used_attacks[magic_index] = attacks[index];
			}
			else {
				fail = true;
			}
		}
		if (!fail) {
			return candidate;
		}
	}
	std::cout << "no number found" << std::endl;
	return 0ULL;
}

void utility::generate_magic_attacks() {
	for (int square = 0; square < 64; square++) {
		uint64_t current_mask = attacks::rook_attack_masks[square];
		int relevant_bits = num_relevant_bits_rook[square];
		int occupancy_indices = 1ULL << relevant_bits;
		for (int i = 0; i < occupancy_indices; i++) {
			uint64_t occupancy = generate_occupancy_by_index(i, relevant_bits, current_mask);
			uint64_t magic_index = (occupancy * magics::rook_magics[square]) >> (64 - relevant_bits);
			attacks::rook_attacks[square][magic_index] = attacks::compute_rook_attacks(square, occupancy);
		}
	}
	for (int square = 0; square < 64; square++) {
		uint64_t current_mask = attacks::bishop_attack_masks[square];
		int relevant_bits = num_relevant_bits_bishop[square];
		int occupancy_indices = 1ULL << relevant_bits;
		for (int i = 0; i < occupancy_indices; i++) {
			uint64_t occupancy = generate_occupancy_by_index(i, relevant_bits, current_mask);
			uint64_t magic_index = (occupancy * magics::bishop_magics[square]) >> (64 - relevant_bits);
			attacks::bishop_attacks[square][magic_index] = attacks::compute_bishop_attacks(square, occupancy);
		}
	}
	return;
}

void utility::generate_magics() {
	for (int i = 0; i < 64; i++) {
		uint64_t magic = generate_magic_number(i, num_relevant_bits_bishop[i], true);
		std::cout << magic << "ULL," << std::endl;
	}
}

/**
 * Returns a random boolean
 *
 * \returns random bool
 */
bool utility::random_bool() {
	return rand() % 2;
}

/**
* Utility function to split a string into a vector.
*
* \param v vector of strings
* \param s string to split
*/

const int utility::num_relevant_bits_bishop[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

const int utility::num_relevant_bits_rook[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

