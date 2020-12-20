#include "trans_table.hpp"

const bool FAST_HEURISTIC = false;

template <class T>
T max4(const T& x0, const T& x1, const T& x2, const T& x3){
    return std::max(std::max(x0, x1), std::max(x2, x3));
}

template <class T>
T max6(const T& x0, const T& x1, const T& x2, const T& x3, const T& x4, const T& x5){
    return std::max(std::max(std::max(x0, x1), std::max(x2, x3)), std::max(x4, x5));
}

// rows are square faces on axis 2 & 3
board_t h_board_0(const board_t& board){return board;}

// rows are square faces on axis 1 & 3
board_t h_board_1(const board_t& board){return swap_1_2(board);}

// rows are square faces on axis 0 & 3
board_t h_board_2(const board_t& board){return swap_0_2(board);}

// rows are square faces on axis 1 & 2
board_t h_board_3(const board_t& board){return swap_1_3(board);}

// rows are square faces on axis 0 & 2
board_t h_board_4(const board_t& board){return swap_0_3(board);}

// rows are square faces on axis 0 & 1
board_t h_board_5(const board_t& board){return transpose(board);}

// function used for parallelization of square face calculations
board_t h_board(const board_t& board, const int& h_idx){
    switch (h_idx){
        case 0:
            return h_board_0(board);
        case 1:
            return h_board_1(board);
        case 2:
            return h_board_2(board);
        case 3:
            return h_board_3(board);
        case 4:
            return h_board_4(board);
        case 5:
            return h_board_5(board);
        default:
            return 0;
    }
}

// function used for parallelization of square face calculations
board_t facet(const board_t& board, const int& f_idx){
    switch (f_idx){
        case 0:
            return board;
        case 1:
            return swap_0_1(board);
        case 2:
            return swap_0_2(board);
        case 3:
            return swap_0_3(board);
        default:
            return 0;
    }
}

trans_table::trans_table(const std::vector<float>& params){
    this->params = params;
    this->params[3] = this->params[3] - 1;
    
    cached_emax_values = std::vector<std::unordered_map<board_t, emax_state>>(MAX_DEPTH);
    
    std::vector<board_t> arr;
    board_t row;
    for (board_t x0 = 0; x0 < 16; ++x0){
        for (board_t x1 = 0; x1 < 16; ++x1){
            for (board_t x2 = 0; x2 < 16; ++x2){
                for (board_t x3 = 0; x3 < 16; ++x3){
                    arr = {x3, x2, x1, x0};
                    row = arr_to_row_transition(arr);
                    
                    _partial_square_row[row] = params[2] * (row_pow_val[row] + params[4] * row_edge_val[row]);

                    _partial_heuristic[row] = (FAST_HEURISTIC ? 3 : 1) * (params[0] * n_row_merges[row] + params[1] * zero_count(arr));
                    
                    _aug_row_mon_vals[row] = params[5] * row_mon_vals[row];
                }
            }
        }
    }
}

// heuristic which encourages all large tiles to be in the same cubic face, square face and edge
float trans_table::cube_score(const board_t& board) const {
    if (FAST_HEURISTIC){
        float square_0_0 = _partial_square_row[ROW_MASK & board];
        float square_0_1 = _partial_square_row[ROW_MASK & (board >> 16)];
        return (square_0_0 + square_0_1) + params[3] * std::max(square_0_0, square_0_1);
    } else {
        board_t board0 = board;
        board_t board1 = h_board_1(board);
        board_t board2 = h_board_3(board);
        
        float square_0_0 = _partial_square_row[ROW_MASK & board0];
        float square_0_1 = _partial_square_row[ROW_MASK & (board0 >> 16)];
        float mon_val_0 = _aug_row_mon_vals[ROW_MASK & board0] + _aug_row_mon_vals[ROW_MASK & (board0 >> 16)];
        
        float square_1_0 = _partial_square_row[ROW_MASK & board1];
        float square_1_1 = _partial_square_row[ROW_MASK & (board1 >> 16)];
        float mon_val_1 = _aug_row_mon_vals[ROW_MASK & board1] + _aug_row_mon_vals[ROW_MASK & (board1 >> 16)];
        
        float square_2_0 = _partial_square_row[ROW_MASK & board2];
        float square_2_1 = _partial_square_row[ROW_MASK & (board2 >> 16)];
        float mon_val_2 = _aug_row_mon_vals[ROW_MASK & board2] + _aug_row_mon_vals[ROW_MASK & (board2 >> 16)];
        
        float facet0 = (square_0_0 + square_0_1) + params[3] * std::max(square_0_0, square_0_1) - mon_val_0;
        float facet1 = (square_0_0 + square_0_1) + params[3] * std::max(square_0_0, square_0_1) - mon_val_0;
        float facet2 = (square_0_0 + square_0_1) + params[3] * std::max(square_0_0, square_0_1) - mon_val_0;
        
        return std::max(std::max(facet0, facet1), facet2);
    }
}

float trans_table::partial_facet_score(const board_t& board) const {
    float cube1 = cube_score(board >> 32);
    float cube2 = cube_score(0xffffffff & board);
    
    return std::max(cube1, cube2) + 0.05 * std::max(cube1, cube2);
    //return std::max(cube_score(board >> 32), cube_score(0xffffffff & board));
}

float trans_table::facet_score(const board_t& board) const {
    if (FAST_HEURISTIC) {
        return std::max(partial_facet_score(board), partial_facet_score(swap_0_1(board)));
    } else {
        return max4(partial_facet_score(board),
        partial_facet_score(swap_0_1(board)),
        partial_facet_score(swap_0_2(board)),
        partial_facet_score(swap_0_3(board)));
    }
}

// the remaining parts of the heuristic
// encourages moving towards boards with merges and blanks and monotone curls in square faces
float trans_table::partial_heuristic(const board_t& board) const {
    return _partial_heuristic[ROW_MASK & board] +
           _partial_heuristic[ROW_MASK & (board >> 16)] +
           _partial_heuristic[ROW_MASK & (board >> 32)] +
           _partial_heuristic[ROW_MASK & (board >> 48)];
}

float trans_table::partial_row_heuristic(const board_t& board) const {
    if (FAST_HEURISTIC) {
        return partial_heuristic(board) + partial_heuristic(transpose(board));
    } else {
        return partial_heuristic(h_board_0(board))
        + partial_heuristic(h_board_1(board))
        + partial_heuristic(h_board_2(board))
        + partial_heuristic(h_board_3(board))
        + partial_heuristic(h_board_4(board))
        + partial_heuristic(h_board_5(board));
    }
}

float trans_table::non_terminal_heuristic(const board_t& board) const {
    return facet_score(board) + partial_row_heuristic(board);
}

float trans_table::heuristic(const board_t& board) const {
    return non_terminal_heuristic(board) - LOSS_PENALTY * is_terminal(board);
}

// move node in expectimax
float trans_table::move_node(const board_t& board, const int& depth, const float& prob, const float& min_prob){
    ++b_eval_count;
    float res = -INFINITY;

    // pick move with greatest expected utility
    int idx = 0;
    u_int16_t move_mask = _valid_move_mask(board);

    // if there are no valid moves, return heuristic
    if (move_mask == 0){
        return heuristic(board);
    }
    
    while (move_mask){
        
        if (move_mask & 1) {
            res = std::max(res, expectation_node(_shift_board(board, DIRECTIONS[idx]), depth, prob, min_prob));
        }
        
        ++idx;
        move_mask >>= 1;
    }

    return res;
}

// expectation node in expectimax
float trans_table::expectation_node(const board_t& board, const int& depth, const float& prob, const float& min_prob){
    std::unordered_map<board_t, emax_state>::iterator address;
    ++b_eval_count;
    
    if ((prob < min_prob) || (depth <= 0)){
        
        // final layer
        // board cannot be terminal if entered from a move node
        return non_terminal_heuristic(board);
        
    } else {
        
        auto address = cached_emax_values[depth].find(board);
        
        // cached expectation layer score
        if (address != cached_emax_values[depth].end()){
            
            {
                // only returns cached values that are calculated to at least the current specified accuracy
                if (address->second.min_prob <= min_prob){

                    return address->second.val;
                }
            }

        }
        
        // uncached expectation layer
        float res = 0;
        board_t free_tiles = is_blank(board);
        
        int n_empty_tiles = popcount(free_tiles);
        float factor = prob / n_empty_tiles;
        board_t randomSetBit = 1;
        
        while (free_tiles){
            if (free_tiles & 1){
                
                // places 2 in free tile
                res += 0.9 * move_node(board | randomSetBit, depth-1, 0.9 * factor, min_prob);
                
                // places 4 in free tile
                res += 0.1 * move_node(board | (randomSetBit << 1), depth-1, 0.1 * factor, min_prob);
                
            }
            
            randomSetBit <<= 4;
            free_tiles >>= 4;
            
        }
        
        res /= n_empty_tiles;
        
        cached_emax_values[depth][board] = {res, min_prob};
        return res;
    }
}

DIRECTION trans_table::expectimax(const Board& board, const int& depth, const float& p_min){
    
    cached_emax_values = std::vector<std::unordered_map<board_t, emax_state>>(MAX_DEPTH);
    
    std::vector<DIRECTION> moves = board.valid_moves();
    assert (moves.size() > 0);
    
    // forces board to make 65536 if it can
    if (_count(board.board, 15) == 2){
        for (auto move : moves){
            if (_count(_shift_board(board.board, move), 15) == 1){
                return move;
            }
        }
    }
    
    // returns argmax
    std::vector<move_state> move_scores;
    
    for (auto move : moves){
        float next_score = expectation_node(_shift_board(board.board, move), depth, 1.0, p_min);
        
        move_scores.push_back({move, next_score});
    }
    
    float best_score = -INFINITY;
    DIRECTION res = moves[0];
    
    for (auto ms : move_scores){
        if (ms.emax_val > best_score){
            best_score = ms.emax_val;
            res = ms.move;
        }
    }
    
    return res;
}


float trans_table::minimax_min(const board_t& board, size_t depth, float alpha, float beta){
    
    float res = INFINITY;
    board_t free_tiles = is_blank(board);
    
    board_t randomSetBit = 1;
    
    while (free_tiles){
        if (free_tiles & 1){
            
            // places 2 in free tile
            res = std::min(res, minimax_max(board | randomSetBit, depth-1, alpha, beta));
            beta = std::min(beta, res);
            if (beta <= alpha) return res;
            
            // places 4 in free tile
            res = std::min(res, minimax_max(board | (randomSetBit << 1), depth-1, alpha, beta));
            beta = std::min(beta, res);
            if (beta <= alpha) return res;

        }
        
        randomSetBit <<= 4;
        free_tiles >>= 4;
    }

    return res;
};

float trans_table::minimax_max(const board_t& board, size_t depth, float alpha, float beta){
    
    float res = -INFINITY;

    // pick move with greatest expected utility
    int idx = 0;
    u_int16_t move_mask = _valid_move_mask(board);

    // if there are no valid moves, return heuristic
    if ((move_mask == 0) or (depth <= 0)){
        return heuristic(board);
    }

    
    while (move_mask){
        
        if (move_mask & 1) {
            res = std::max(res, minimax_min(_shift_board(board, DIRECTIONS[idx]), depth, alpha, beta));
        }
        
        ++idx;
        move_mask >>= 1;
    }

    return res;
    
};

DIRECTION trans_table::minimax(const Board& board, size_t depth){
    std::vector<DIRECTION> moves = board.valid_moves();
    assert (moves.size() > 0);
    
    // forces board to make 65536 if it can
    if (_count(board.board, 15) == 2){
        for (auto move : moves){
            if (_count(_shift_board(board.board, move), 15) == 1){
                return move;
            }
        }
    }
    
    // returns argmax
    std::vector<move_state> move_scores;
    
    for (auto move : moves){
        float next_score = minimax_min(_shift_board(board.board, move), depth);
        move_scores.push_back({move, next_score});
    }
    
    float best_score = -INFINITY;
    DIRECTION res = moves[0];
    
    for (auto ms : move_scores){
        if (ms.emax_val > best_score){
            best_score = ms.emax_val;
            res = ms.move;
        }
    }
    
    return res;
};

DIRECTION trans_table::mcts(const Board& board, size_t n_sims){
    std::vector<DIRECTION> moves = board.valid_moves();
    assert (moves.size() > 0);
    
    int best_score = 0;
    DIRECTION res = moves[0];
    
    for (auto move : moves){
        
        int tmp_score = 0;
        
        for (int i = 0; i < n_sims/moves.size(); ++i){
            Board tmp_board = board;
            tmp_board.move(move);
            tmp_board.generate_piece();
            
            while (!is_terminal(tmp_board.board)){
                tmp_board.move(tmp_board.random_move());
            }
            tmp_score += tmp_board.score();
        }
        if (tmp_score > best_score){
            best_score = tmp_score;
            res = move;
        }
    }
    return res;
}
