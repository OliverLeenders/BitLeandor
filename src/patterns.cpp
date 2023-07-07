#include "patterns.h"

void patterns::init_patterns() {
    // initialize manhattan distance table
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            manhattan_distance[i][j] = std::abs((i / 8) - (j / 8)) + std::abs((i % 8) - (j % 8));
        }
    }

    // initialize spans
    for (int i = 0; i < 64; i++) {
        int forward = i + 8;
        int backward = i - 8;
        uint64_t front_span = 0ULL;
        for (; forward < 64; forward += 8) {
            front_span |= 1ULL << forward;
        }
        uint64_t back_span = 0ULL;
        for (; backward >= 0; backward -= 8) {
            back_span |= 1ULL << backward;
        }
        front_spans[WHITE][i] = front_span;
        front_spans[BLACK][i] = back_span;
    }

    for (int i = 0; i < 64; i++) {
        uint64_t front_span = 0ULL;
        uint64_t back_span = 0ULL;
        if (i % 8 != 0) {
            int forward = i + 7;
            int backward = i - 9;
            for (; forward < 64; forward += 8) {
                front_span |= 1ULL << forward;
            }
            for (; backward >= 0; backward -= 8) {
                back_span |= 1ULL << backward;
            }
        }
        if (i % 8 != 7) {
            int forward = i + 9;
            int backward = i - 7;
            for (; forward < 64; forward += 8) {
                front_span |= 1ULL << forward;
            }
            for (; backward >= 0; backward -= 8) {
                back_span |= 1ULL << backward;
            }
        }
        attack_front_spans[WHITE][i] = front_span;
        attack_front_spans[BLACK][i] = back_span;
    }

	// init neighbour files array
    for (int i = 0; i < 8; i++) {
        uint64_t neighbour = 0ULL;
        if (i < 7) {
            neighbour |= files[i + 1];
        }
		if (i > 0) {
			neighbour |= files[i - 1];
		}
		neighbour_files[i] = neighbour;
    }
}

int patterns::manhattan_distance[NUM_SQUARES][NUM_SQUARES] = {{0}};

// clang-format off
const int patterns::center_manhattan_distance[NUM_SQUARES] = {
	6, 5, 4, 3, 3, 4, 5, 6,
	5, 4, 3, 2, 2, 3, 4, 5,
	4, 3, 2, 1, 1, 2, 3, 4,
	3, 2, 1, 0, 0, 1, 2, 3,
	3, 2, 1, 0, 0, 1, 2, 3,
	4, 3, 2, 1, 1, 2, 3, 4,
	5, 4, 3, 2, 2, 3, 4, 5,
	6, 5, 4, 3, 3, 4, 5, 6
};

// clang-format on

uint64_t patterns::front_spans[NUM_COLORS][NUM_SQUARES] = {{0ULL}};
uint64_t patterns::attack_front_spans[NUM_COLORS][NUM_SQUARES] = {{0ULL}};
uint64_t patterns::neighbour_files[8] = {0ULL};
