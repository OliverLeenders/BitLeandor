#pragma once
#include <cstdint>

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