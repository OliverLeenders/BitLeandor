#pragma once

#include "bitboard_util.h"
#include "magics.h"

class attacks {
  public:
    static uint64_t knight_attacks[64];
    static uint64_t pawn_attacks[2][64];
    static uint64_t king_attacks[64];
    static uint64_t rook_attack_masks[64];
    static uint64_t bishop_attack_masks[64];
    static uint64_t rook_magics[64];
    static uint64_t bishop_magics[64];
    static uint64_t squares_between[64][64];
    static uint64_t bishop_attacks[64][512];
    static uint64_t rook_attacks[64][4096];

    static uint64_t diagonal_masks[64];
    static uint64_t horizontal_vertical_masks[64];

    static void init_directional_masks();
    static void init_squares_between();
    static void init_attack_tables();
    static void init_knight_attacks();
    static void init_pawn_attacks();
    static void init_king_attacks();
    static void init_rook_attack_masks();
    static uint64_t compute_rook_attacks(int square, uint64_t blockers);
    static uint64_t compute_bishop_attacks(int square, uint64_t blockers);
    static void init_bishop_attack_masks();
    static const int64_t num_relevant_bits_rook[64];
    static const int64_t num_relevant_bits_bishop[64];

    /**
     * @brief Get the rook attacks. Works via fixed shift plain magic bitboards.
     *
     * @param square where the piece is positioned (rook, queen)
     * @param occupancy the occupancy bitboard (black AND white)
     * @return all squares attacked (vertical & horizontal) by the piece
     */
    static inline uint64_t get_rook_attacks(int square, uint64_t occupancy) {
        occupancy &= rook_attack_masks[square];
        occupancy *= magics::rook_magics[square];
        occupancy >>= 64 - num_relevant_bits_rook[square];

        return rook_attacks[square][occupancy];
    }

    /**
     * @brief Get the bishop attacks. Works via fixed shift plain magic bitboards.
     *
     * @param square where the piece is positioned (bishop, queen)
     * @param occupancy the occupancy bitboard (black AND white)
     * @return all squares attacked (diagonally) by the piece
     */
    static inline uint64_t get_bishop_attacks(int square, uint64_t occupancy) {
        occupancy &= bishop_attack_masks[square];
        occupancy *= magics::bishop_magics[square];
        occupancy >>= 64 - num_relevant_bits_bishop[square];

        return bishop_attacks[square][occupancy];
    }

    /**
     * @brief Get the rook xray-attacks.
     * TODO: use this somehow
     *
     * @param square where the piece is positioned (rook, queen)
     * @param occupancy the occupancy bitboard (black AND white)
     * @param blockers all squares that may block a (horizontal or vertical) attack
     * @return all squares that are x-rayed by the piece
     */
    static inline uint64_t get_rook_xray_attacks(int square, uint64_t occupancy, uint64_t blockers) {
        uint64_t attacks = get_rook_attacks(square, occupancy);
        blockers &= attacks;

        return attacks ^ get_rook_attacks(square, blockers ^ occupancy);
    }

    /**
     * @brief Get the bishop xray-attacks.
     * TODO: use this somehow
     *
     * @param square where the piece is positioned (bishop, queen)
     * @param occupancy the occupancy bitboard (black AND white)
     * @param blockers all squares that may block a (diagonal) attack
     * @return all squares that are x-rayed by the piece
     */
    static inline uint64_t get_bishop_xray_attacks(int square, uint64_t occupancy, uint64_t blockers) {
        uint64_t attacks = get_bishop_attacks(square, occupancy);
        blockers &= attacks;

        return attacks ^ get_bishop_attacks(square, blockers ^ occupancy);
    }
};
