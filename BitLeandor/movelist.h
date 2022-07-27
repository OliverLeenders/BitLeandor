#pragma once

#include "bit_move.h"

class movelist
{
public:
	movelist();
	bit_move moves[265] = {};
	int size = 0;
	~movelist();

private:

};

