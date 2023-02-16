#include "perft.h"
/// <summary>
/// Recursive perft function
/// </summary>
/// <param name="b"></param>
/// <param name="depth"></param>
/// <returns></returns>
uint64_t perft::run_perft(bitboard* b, int depth)
{
	uint64_t nodes = 0;
	if (depth == 0)
	{
		return 1;
	}
	else
	{
		movelist l;
		movegen::generate_all_pseudo_legal_moves(b, &l);
		for (int i = 0; i < l.size; i++)
		{
			bit_move m = l.moves[i];
			if (b->is_legal<true>(&m))
			{
				b->make_move(&m);
				//std::cout << bit_move::to_string(m) << std::endl;
				nodes += run_perft(b, depth - 1);
				b->unmake_move();
			}
		}
		return nodes;
	}

}

/// <summary>
/// Perft test for a given position and depth 
/// prints the number of nodes at each depth for each move
/// </summary>
/// <param name="b">the position </param>
/// <param name="depth">perft depth</param>
/// <returns></returns>
uint64_t perft::run_perft_console(bitboard* b, int depth)
{
	uint64_t nodes = 0;
	if (depth == 0)
	{
		return 1;
	}
	else
	{
		movelist l;
		movegen::generate_all_pseudo_legal_moves(b, &l);
		bit_move m;
		for (int i = 0; i < l.size; i++)
		{
			m = l.moves[i];
			if (b->is_legal<true>(&m))
			{
				std::cout << bit_move::to_string(m) << ": ";
				
				b->make_move(&m);
				
				uint64_t curr_nodes = run_perft(b, depth - 1);
				nodes += curr_nodes;
				std::cout << curr_nodes << std::endl;
				b->unmake_move();
			}
			else {
				// std::cout << "\t[" << bit_move::to_string(m) << "] (" << (int)m.get_flags() << "): " << std::endl;
				// b->print_board();
				// std::cout << std::endl;
			}
		}
		std::cout << "Total nodes: " << nodes << std::endl;

		return nodes;
	}
}

