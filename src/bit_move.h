#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

class bit_move {
  public:
    uint32_t move = 0U;

    /**
     * @brief Construct a new bit move object
     *
     * @param origin the origin square (0-63)
     * @param target the target square (0-63)
     * @param flags the move flags
     * @param type the piece type
     * @param captured_type the captured piece type
     */
    bit_move(uint8_t origin, uint8_t target, uint8_t flags, uint8_t type, uint8_t captured_type);
    /**
     * `0000 0000 0000 00000000 00000000`
     * `t... ct.. fl.. target.. origin`
     */
    bit_move() = default;
    bit_move(const bit_move &other) = default;
    bit_move &operator=(const bit_move &other) = default;
    bit_move(bit_move &&other) = default;
    bit_move &operator=(bit_move &&other) = default;

    uint8_t get_origin();
    uint8_t get_target();
    uint8_t get_flags();
    uint8_t get_captured_type();
    uint8_t get_piece_type();

    enum flags : uint8_t {
        quiet_move,
        double_pawn_push,
        kingside_castle,
        queenside_castle,
        capture,
        ep_capture,
        knight_promotion = 8U,
        bishop_promotion,
        rook_promotion,
        queen_promotion,
        knight_capture_promotion,
        bishop_capture_promotion,
        rook_capture_promotion,
        queen_capture_promotion
    };
    static const std::vector<uint8_t> flag_list;
    static inline bool is_capture(bit_move *m) { return m->get_flags() & flags::capture; }
    static inline bool is_promotion(bit_move *m) { return m->get_flags() >= knight_promotion; }

    inline bool operator==(const bit_move &rhs) { return this->move == rhs.move; };
    inline bool operator!=(const bit_move &rhs) { return this->move != rhs.move; };

    static constexpr uint32_t NULL_MOVE = 0U;
    static std::string to_string(bit_move m);
    static std::string to_long_string(bit_move m);
    static std::string squares_to_string[64];
};

static_assert(std::is_trivially_copyable<bit_move>::value, "bit_move must be trivially copyable");
