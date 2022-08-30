#include "tt_entry.h"

tt_entry::tt_entry()
{
}

tt_entry::tt_entry(uint64_t key, int set_score, int set_depth, int set_type, bit_move set_move)
{
	this->score = set_score;
	this->depth = set_depth;
	this->type = set_type;
	this->hash_move = set_move;
}

tt_entry::~tt_entry()
{
}
