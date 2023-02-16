#pragma once
#include <cstdint>

inline int MATE = 100000;
enum Colors : uint8_t{
	WHITE = 0,
	BLACK = 1
};

enum Types : uint8_t {
	PAWN = 0,
	KNIGHT = 1,
	BISHOP = 2,
	ROOK = 3,
	QUEEN = 4,
	KING = 5,
	EMPTY = 6
};

enum Pieces : uint8_t {
	WHITE_PAWN = 0,
	WHITE_KNIGHT = 1,
	WHITE_BISHOP = 2,
	WHITE_ROOK = 3,
	WHITE_QUEEN = 4,
	WHITE_KING = 5,
	BLACK_PAWN = 6,
	BLACK_KNIGHT = 7,
	BLACK_BISHOP = 8,
	BLACK_ROOK = 9,
	BLACK_QUEEN = 10,
	BLACK_KING = 11,
	EMPTY_PIECE = 12
};

enum Castling_Rights : uint8_t {
	WHITE_KINGSIDE = 0,
	WHITE_QUEENSIDE = 1,
	BLACK_KINGSIDE = 2,
	BLACK_QUEENSIDE = 3
};

enum Squares : uint8_t {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	NUM_SQUARES
};
