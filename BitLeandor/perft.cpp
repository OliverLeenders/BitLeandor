#include "perft.h"

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
				std::cout << i << ".\t" << bit_move::to_string(m) + " (" << (int)m.get_flags() << "): ";
				b->make_move(&m);
				uint64_t curr_nodes = run_perft(b, depth - 1);
				nodes += curr_nodes;
				std::cout << curr_nodes << std::endl;
				b->unmake_move();
			}
			else {
				std::cout << bit_move::to_string(m) + "(" << (int)m.get_flags() << "): illegal" << std::endl;
			}
		}
		std::cout << "Total nodes: " << nodes << std::endl;

		return nodes;
	}
}

uint64_t perft::run_two_level_perft(bitboard* b, int depth_extensive, int depth, int indentation)
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
				std::string indent = "\n";
				for (int j = 0; j < indentation; j++)
				{
					indent += "    ";
				}
				std::cout << indent << bit_move::to_string(m) + "(" 
					<< (int)m.get_flags() << ", " 
					<< (int)m.get_origin() << ", " 
					<< (int)m.get_target() << ", " 
					<< (int)m.get_piece_type() << ", " 
					<< (int)m.get_captured_type() << "): ";
				b->make_move(&m);

				uint64_t curr_nodes = 0;
				if (depth_extensive > 2 && ((indentation == 0 && i == 0) || (indentation > 0))) {
					std::cout << std::endl;
					bitboard_util::print_bitboard(b->pieces[2]);
					// curr_nodes = run_two_level_perft(b, depth_extensive - 1, depth - 1, indentation + 1);
				}
				// else {
				// 	curr_nodes = run_perft(b, depth - 1);
				// }*/
				curr_nodes = run_perft(b, depth - 1);
				nodes += curr_nodes;
				std::cout << curr_nodes << std::endl;
				b->unmake_move();
			}
		}
		return nodes;
	}
}
