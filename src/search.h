#pragma once
#include <chrono>
#include <io.h>

#include "bitboard.h"
#include "evaluator.h"
#include "movegen.h"
#include "movelist.h"
#include "transposition_table.h"
#include "uci_io.h"

class search {
  public:
    //============================================================================================//
    //
    // (static) member variables
    //
    //============================================================================================//

    /**
     * @brief The time point (approx.) when the current search started.
     */
    static std::chrono::time_point<std::chrono::high_resolution_clock> ENDTIME;

    /**
     * @brief The time point (approx.) when the current search should end (only if the search is
     * time limited and not depth or nodes limited).
     */
    static std::chrono::time_point<std::chrono::high_resolution_clock> STARTTIME;

    /**
     * @brief The maximum size of the principal variation. (= MAX_DEPTH, MAX_PLY)
     */
    static constexpr int MAX_PV_SIZE = 256;

    /**
     * @brief The maximum depth to search to (= MAX_PV_SIZE, MAX_PLY)
     */
    static constexpr int MAX_DEPTH = 265;

    /**
     * @brief The maximum length of any path from some position to the root position of the search.
     * (= MAX_PV_SIZE, MAX_DEPTH)
     */
    static constexpr int MAX_PLY = 256;
    static constexpr int CAPTURE_SCORE = 100000;
    static constexpr int KILLER_1_SCORE = 10000;
    static constexpr int KILLER_2_SCORE = 9999;
    static int lmr[MAX_PV_SIZE][MAX_PV_SIZE];

    /**
     * @brief the depth (distance from root) to search to
     */
    static int DEPTH;

    /**
     * @brief the selective search depth, i.e. the maximum distance from root searched with an open
     * window (=> not a zero-window). Can be greater than DEPTH but never smaller.
     */
    static int SELDEPTH;

    /** TODO: check if history overflows */
    static int history[2][64][64];

    // Principal Variation array
    static bit_move PV[MAX_PLY][MAX_PV_SIZE];
    static int PV_SIZE[MAX_DEPTH];
    static int prev_pv_size;

    static int NODES_SEARCHED;
    static int QNODES_SEARCHED;

    // clang-format off
    static constexpr int mvv_lva[6][6] = {
        {105, 205, 305, 405, 505, 605},
        {104, 204, 304, 404, 504, 604},
        {103, 203, 303, 403, 503, 603},
        {102, 202, 302, 402, 502, 602},
        {101, 201, 301, 401, 501, 601},
        {100, 200, 300, 400, 500, 600}
    };
    // clang-format on

    /**
     * @brief The array of movelists, one movelist per possible depth from root (0 - 255).
     */
    static movelist moves[256];
    static bit_move killers[2][MAX_PV_SIZE];

    /**
     * @brief The main transposition table. Contains scores of positions previously searched that
     * can be retrieved using their zobrist-hash key.
     */
    static transposition_table tt;

    /**
     * @brief indicator for wether the search should be aborted. Is set to true if either the time
     * is up or the search is aborted from the console.
     */
    static bool stop_now;

    //============================================================================================//
    //
    // (static) member functions
    //
    //============================================================================================//

    /**
     * @brief Performs an iterative deepening PVS alpha-beta search.
     *
     * @param b board representation
     * @param depth depth to search to
     * @param quiet flag indicating wether search info should be logged to stdout
     * @return Total number of nodes searched
     */
    static int search_iterative_deepening(bitboard *b, int depth, bool quiet);

    /**
     * @brief Implementation of alpha-beta pruning function. Searches until time has
     * elapsed or until given depth has been reached.
     *
     * @param b the board representation in order to make moves and evaluate positions
     * @param depth the maximum depth to search to
     * @param alpha lower bound for alpha-beta pruning
     * @param beta upper bound for alpha-beta pruning
     * @param ply the current ply from the root of the search-tree
     * @return the score of the position
     */
    static int alpha_beta(bitboard *b, int depth, int alpha, int beta, int ply);

    /**
     * @brief Performs a quiescence search (searching only captures) until no captures are possible
     * anymore. When there is a check, all moves -- not only captures -- are searched.
     *
     * @param b the board representation
     * @param alpha lower-bound for alpha-beta pruning
     * @param beta upper-boud for alpha-beta pruning
     * @param ply the ply from the root of the search tree
     * @return score of the position
     */
    static int quiescence(bitboard *b, int alpha, int beta, int ply);

    /**
     * @brief Updates the PV table.
     *
     * The PV table is updated by copying the PV from the previous iteration
     * and adding the new move to the beginning of the PV.
     *
     * @param m The move to add to the PV
     * @param ply The current ply
     */
    static void update_PV(bit_move *m, int depth);

    /**
     * @brief Iterates over move list and scores them according to their respective
     * move-ordering scores.
     *
     * Captures are scored with MVV-LVA scoring, non-captures are sorted with scores
     * from the history-heuristic.
     *
     * @param m_l Movelist
     * @param s_l Scorelist
     * @param side_to_move Side to move
     */
    static void score_moves(movelist *m_l, bool side_to_move);

    /**
     * @brief Prints the values of the movelist struct.
     *
     * @param m_l the movelist to be printed
     */
    static void print_move_list(movelist *m_l);

    /**
     * @brief Swaps the move with the highest score in the move list to the back of the list. Also
     * swaps the scores. Basically a selection sort.
     *
     * @param m_l Movelist
     * @param s_l Scorelist
     * @param index Index pointing to next element after sorted area
     */
    static void fetch_next_move(movelist *m_l, int index);

    /**
     * @brief Initializes the values used for late-move-reductions. [Currently not used].
     */
    static void init_lmr();

    /**
     * @brief resets all entrires of the history table to 0.
     */
    static void clear_history();

    /**
     * @brief resets both killer move entries to nullmoves.
     */
    static void clear_killers();

    /**
     * @brief Clears the principal variation.
     */
    static void clear_PV();

    /**
     * @brief Iterates over the PV table and prints the PV to stdout.
     * When the PV contains a null move it should fail gently.
     * When the PV contains an illegal move, it should report this to stdout.
     *
     * It also rehashes the PV nodes into the Transposition Table.
     *
     * @param b board representation
     * @param score the score determined by the search
     * @param curr_depth the depth of the search at the point of calling this
     */

    static void gather_and_print_pv(bitboard *b, int score, int curr_depth);

    /**
     * @brief Checks if search time is up and checks for user input that would trigger search
     * abortion.
     */
    static void communicate();
};
