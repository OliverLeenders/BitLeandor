#include "bit_move.h"

bit_move::bit_move(uint8_t origin, uint8_t target, uint8_t flags, uint8_t type, uint8_t captured_type) {
    this->move = ((type & 15) << 20)            // shift the piece type to the correct position
                 | ((captured_type & 15) << 16) // shift the captured type ...
                 | ((flags & 15) << 12)         // shift the flags
                 | ((origin & 63) << 6)         // shift the origin
                 | (target & 63);               // target need not be shifted
}

bit_move::bit_move() { this->move = 0; }

bit_move::~bit_move() {}

uint8_t bit_move::get_origin() { return (this->move >> 6) & 63; }

uint8_t bit_move::get_target() { return this->move & 63; }

uint8_t bit_move::get_flags() { return (this->move >> 12) & 15; }

uint8_t bit_move::get_captured_type() { return (this->move >> 16) & 15; }

uint8_t bit_move::get_piece_type() { return (this->move >> 20); }

std::string bit_move::to_string(bit_move m) {
    std::string s = squares_to_string[m.get_origin()] + squares_to_string[m.get_target()];
    uint16_t flags = m.get_flags();
    if (flags >= 8) { // if it is promotion
        uint8_t last_digits = flags & 3U;
        switch (last_digits) {
        case 0:
            s += "n";
            break;
        case 1:
            s += "b";
            break;
        case 2:
            s += "r";
            break;
        case 3:
            s += "q";
            break;
        default:
            break;
        }
    }
    return s;
}

// clang-format off
std::string bit_move::squares_to_string[64] = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};
// clang-format on
