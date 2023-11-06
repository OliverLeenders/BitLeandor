#pragma once
#include <chrono>
#include <cstdint>

#include "bitboard.h"
#include "evaluator.h"
#include "movegen.h"
#include "movelist.h"

class perft {
  public:
    /**
     * @brief Recursive perft test function
     *
     * @param b board representation
     * @param depth depth left
     *
     * @return number of positions
     */
    static uint64_t run_perft(bitboard *b, int depth);

    /**
     * @brief Top-level perft test function. Prints perft info to the console
     *
     * @param b board representation
     * @param depth depth to search
     *
     * @return number of leaf nodes
     */
    static uint64_t run_perft_console(bitboard *b, int depth);

    /**
     * @brief Top-level perft test function. Prints perft info to the console. Differs from 
     * run_perft in that a staged move generator is used.
     *
     * @param b board representation
     * @param depth depth to search
     *
     * @return number of leaf nodes
     */
    static uint64_t run_perft_staged(bitboard *b, int depth, int ply);


    static uint64_t run_perft_staged_console(bitboard *b, int depth);
};
