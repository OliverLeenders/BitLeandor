#pragma once

#include "cmath"
#include "constants.h"
/**
 * TODO: Refactor such that all patterns in bitboard_util.h are moved here!
 */
#include "bitboard_util.h"

class patterns {
  public:
    static void init_patterns();
    // distance arrays
    static int manhattan_distance[NUM_SQUARES][NUM_SQUARES];
    const static int center_manhattan_distance[64];

    static uint64_t neighbour_files[8];
    static uint64_t front_spans[NUM_COLORS][NUM_SQUARES];
    static uint64_t attack_front_spans[NUM_COLORS][NUM_SQUARES];
};
