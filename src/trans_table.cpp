#include "trans_table.hpp"

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

trans_table::trans_table(const std::vector<float>& params){
    this->params = params;
    cached_expectimax_values = std::vector<std::unordered_map<board_t, float>>(MAX_DEPTH);
    
    std::vector<board_t> arr;
    board_t row;
    for (board_t x0 = 0; x0 < 16; ++x0){
        for (board_t x1 = 0; x1 < 16; ++x1){
            for (board_t x2 = 0; x2 < 16; ++x2){
                for (board_t x3 = 0; x3 < 16; ++x3){
                    arr = {x3, x2, x1, x0};
                    row = arr_to_row_transition(arr);
                    
                    _partial_square_row[row] = row_pow_val[row] + (params[4]/(params[3] + 1e-8)) * row_edge_val[row];
                    _partial_heuristic[row] = params[0] * n_row_merges[row] + params[1] * zero_count(arr) - params[5] * row_mon_vals[row];
                }
            }
        }
    }
}

// heuristic which encourages all large tiles to be in the same cubic face
float trans_table::cube_score(const board_t& board) const {
    
    board_t board0 = board;
    board_t board1 = h_board_1(board);
    board_t board2 = h_board_3(board);
    
    float facet0 = _partial_square_row[ROW_MASK & board0] + _partial_square_row[ROW_MASK & (board0 >> 16)];
    float facet1 = _partial_square_row[ROW_MASK & board1] + _partial_square_row[ROW_MASK & (board1 >> 16)];
    float facet2 = _partial_square_row[ROW_MASK & board2] + _partial_square_row[ROW_MASK & (board2 >> 16)];

    return std::max(std::max(facet0, facet1), facet2);
}

float trans_table::partial_facet_score(const board_t& board) const {
    float facet1 = cube_score(0xffffffff & board);
    float facet2 = cube_score(board >> 32);
    return std::max(facet1, facet2);
}

float trans_table::facet_score(const board_t& board) const {
    return max4(partial_facet_score(board),
    partial_facet_score(swap_0_1(board)),
    partial_facet_score(swap_0_2(board)),
    partial_facet_score(swap_0_3(board)));
}

// heuristic which encourages largest tiles to be in the same square face
float trans_table::partial_square_score(const board_t& board) const {
    return max4(_partial_square_row[ROW_MASK & board],
    _partial_square_row[ROW_MASK & (board >> 16)],
    _partial_square_row[ROW_MASK & (board >> 32)],
    _partial_square_row[ROW_MASK & (board >> 48)]);
}

float trans_table::square_score(const board_t& board) const {
    return max6(partial_square_score(h_board_0(board)),
    partial_square_score(h_board_1(board)),
    partial_square_score(h_board_2(board)),
    partial_square_score(h_board_3(board)),
    partial_square_score(h_board_4(board)),
    partial_square_score(h_board_5(board)));
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
    return partial_heuristic(h_board_0(board))
    + partial_heuristic(h_board_1(board))
    + partial_heuristic(h_board_2(board))
    + partial_heuristic(h_board_3(board))
    + partial_heuristic(h_board_4(board))
    + partial_heuristic(h_board_5(board));
}

float trans_table::heuristic(const board_t& board) const {
    return params[2] * facet_score(board)
    + params[3] * square_score(board)
    + partial_row_heuristic(board)
    - LOSS_PENALTY * is_terminal(board);
}

// move node in expectimax
float trans_table::move_node(const board_t& board, const int& depth, const float& prob, const float& min_prob){
    
    float res = -INFINITY;

    // pick move with greatest expected utility
    int idx = 0;
    u_int16_t move_mask = _valid_move_mask(board);

    // if there are no valid moves, return heuristic
    if (move_mask == 0) return heuristic(board);
    
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
    std::unordered_map<board_t, float>::iterator address;
    
    if ((prob < min_prob) || (depth <= 0)){
        
        // final layer
        return heuristic(board);
        
    } else if ((address = cached_expectimax_values[depth].find(board)) != cached_expectimax_values[depth].end()){
        
        // cached expectation layer score
        return address->second;
        
    } else {
        
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
        cached_expectimax_values[depth][board] = res;
        return res;
    }
}

DIRECTION trans_table::expectimax(const Board& board, const int& depth, const float& p_min){
    
    cached_expectimax_values = std::vector<std::unordered_map<board_t, float>>(MAX_DEPTH);
    
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
    float best_score = -INFINITY;

    DIRECTION res = moves[0];
    
    for (auto move : moves){
        float next_score = expectation_node(_shift_board(board.board, move), depth, 1.0, p_min);
    
        if (next_score > best_score){
            best_score = next_score;
            res = move;
        }
    }
    
    return res;
}
