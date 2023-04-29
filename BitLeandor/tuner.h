#pragma once
#include "evaluator.h"
#include "movegen.h"
#include "utility.h"
#include <fstream>
#include <cmath>

class tuner
{
public:
	// 
	static bitboard b;
	
	// entry representing a position for the tuner
	// * does not 
	
	static int weights[6 + 6 + 64 * 14 + 8 * 2 + 2];
	static uint16_t w_pieces_MG_idx;
	static uint16_t w_pieces_EG_idx;
	static uint16_t w_PST_MG_idx;
	static uint16_t w_PST_EG_idx;
	static uint16_t w_pp_bonus_MG_idx;
	static uint16_t w_pp_bonus_EG_idx;
	static uint16_t w_dp_penalty_MG_idx;
	static uint16_t w_dp_penalty_EG_idx;
	static uint16_t w_pawn_shield_penalty_idx;

	/**
	 * @brief tuple indicating a used evaluation feature.
	 * @li sad
	 * @li asd
	*/
	struct ttuple {
		uint16_t index = 0;
		int8_t white_coefficient, black_coefficient = 0;
	};
	struct tuner_entry {
		int static_eval = 0;
		int eval = 0;
		bitboard b = bitboard();
		float phase = 0.0f;
		float result = 0.0f;
		std::vector<ttuple> used_weights = std::vector<ttuple>();
	};
	static double mean_square_error(double k, int num_positions, std::ifstream* f);
	static double mean_square_error(double k, std::vector<tuner_entry>* entries);

	static void tune(double k);
	static void init_coefficients(bitboard* b, std::vector<ttuple>* tuples);
	static void init_weights();
	static float parse_result_from_epd(std::string epd_line);
	static void filter_quiet_positions();
};

