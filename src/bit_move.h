#pragma once

#include <cstdint>
#include <string>
#include <vector>

class bit_move {
  public:
    uint32_t move = 0U;

    bit_move(uint8_t origin, uint8_t target, uint8_t flags, uint8_t type, uint8_t captured_type);
    /**
     * `0000 0000 0000 00000000 00000000`
     * `t... ct.. fl.. target.. origin`
     */
    bit_move();
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
    ~bit_move();
    static std::string squares_to_string[64];
};
