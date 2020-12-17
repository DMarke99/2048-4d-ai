#pragma once
#include "board.hpp"
#include "trans_table.hpp"
#include <stdio.h>
#include <chrono>
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>

void display_ai_game(int depth, float min_prob);
void test_params(int depth, float min_prob, size_t n_sims);
float test_transition(int depth, float min_prob, board_t initial_pos, size_t terminal_rank, size_t n_gens, size_t n_sims);
