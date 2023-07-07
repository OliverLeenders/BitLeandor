#pragma once
#include "bit_move.h"

class tt_entry {
  public:
    static const int VAL_UNKNOWN = 200000;

    uint64_t key = 0ULL;
    bit_move hash_move = bit_move();
    int score = VAL_UNKNOWN;
    uint8_t depth = 0;
    uint8_t type = NONE;

    /**
     * @brief Construct a new tt entry object
     */
    tt_entry();

    /**
     * @brief Destroy the tt entry object
     */
    ~tt_entry();

    /**
     * @brief Construct a new tt entry object
     *
     * @param key the key of the entry
     * @param set_score the score
     * @param set_depth the depth this entry has been searched to
     * @param set_type the type [exact, upper_bound, lowerbound]
     * @param set_move the best move
     */
    tt_entry(uint64_t key, int set_score, int set_depth, int set_type, bit_move set_move);

    enum { EXACT = 0, LOWER_BOUND = 1, UPPER_BOUND = 2, NONE = 3 };
};
