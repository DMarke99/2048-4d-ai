#include "trans_table.hpp"

const size_t MAX_DEPTH = 16;
const bool REORGANIZE_BOARD = true;
const bool FAST_HEURISTIC = false;
const bool ALL_CUBES = true;
const bool MULTITHREADED = true;

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

// rotates and flips board in 4d to an equivalent board to make it easier to calculate heuristic
board_t reorganize(const board_t& board, const size_t& level){
    board_t res = board;
    size_t tmp_size;
    board_t tmp_board;
    switch (level){
            
        // Corner optimisation
        case 0: {
            
            // finds location of highest value piece
            size_t i = 15;
            size_t max_rank = 0;
            size_t max_rank_loc = 0;
            for (board_t tmp = res; tmp; tmp >>= 4, --i){
                if ((tmp_size = tmp & 0xf) > max_rank){
                    max_rank = tmp_size;
                    max_rank_loc = i;
                }
            }
            
            // moves highest value piece to left-most bit
            int idx = 0;
            for (; max_rank_loc; max_rank_loc >>= 1, ++idx){
                if (max_rank_loc & 1) res = flip(res, 3-idx);
            }
        }
            
        // Edge rearrangement
        case 1: {
            
            // finds location of highest value piece adjacent to location [0,0,0,0]
            size_t second_rank = (res >> 56) & 0xf;
            size_t second_rank_loc = 1;
            
            if ((tmp_size = (res >> 52) & 0xf) > second_rank){
                second_rank = tmp_size;
                second_rank_loc = 2;
            }
            
            if ((tmp_size = (res >> 44) & 0xf) > second_rank){
                second_rank = tmp_size;
                second_rank_loc = 4;
            }
            
            if ((tmp_size = (res >> 28) & 0xf) > second_rank){
                second_rank = tmp_size;
                second_rank_loc = 8;
            }
            
            // moves piece to second-left-most bit
            switch (second_rank_loc) {
                case 2: res = swap_2_3(res); break;
                case 4: res = swap_1_3(res); break;
                case 8: res = swap_0_3(res); break;
                default: break;
            }
        }
            
        // Square rearrangement
        case 2: {
            
            // finds highest value edge that forms a square with [0,0,0,.] and rotates it into edge [0,0,1,.]
            size_t edge_val = row_val[(res >> 48) & 0xff];
            size_t edge_idx = 0;
            
            if ((tmp_board = row_val[(res >> 40) & 0xff]) > edge_val){
                edge_val = tmp_board;
                edge_idx = 1;
            }
        
            if ((tmp_board = row_val[(res >> 24) & 0xff]) > edge_val){
                edge_val = tmp_board;
                edge_idx = 2;
            }
            
            // moves piece to second-left-most bit
            switch (edge_idx) {
                case 1: res = swap_1_2(res); break;
                case 2: res = swap_0_2(res); break;
                default: break;
            }
        }
            
        // Cube rearrangement
        case 3: {
            
            // finds highest value square that forms a cube with [0,0,.,.] and rotates it into square [0,1,.,.]
            if (row_val[(res >> 16) & 0xffff] > row_val[(res >> 32) & 0xffff]) res = swap_0_1(res);
            return res;
        };
        default: return res;
    }
}

trans_table::trans_table(const std::vector<float>& params) : b_eval_count(0) {
    this->params = params;
    if (!REORGANIZE_BOARD) this->params[3] = this->params[3] - 1;
    
    std::vector<board_t> arr;
    board_t row;
    for (board_t x0 = 0; x0 < 16; ++x0){
        for (board_t x1 = 0; x1 < 16; ++x1){
            for (board_t x2 = 0; x2 < 16; ++x2){
                for (board_t x3 = 0; x3 < 16; ++x3){
                    arr = {x3, x2, x1, x0};
                    row = arr_to_row_transition(arr);
                    
                    _partial_square_row[row] = params[2] * (row_pow_val[row] + params[4] * row_edge_val[row]);
                    _aug_partial_square_row[row] = params[3] * _partial_square_row[row];
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
        float facet1 = (square_1_0 + square_1_1) + params[3] * std::max(square_1_0, square_1_1) - mon_val_1;
        float facet2 = (square_2_0 + square_2_1) + params[3] * std::max(square_2_0, square_2_1) - mon_val_2;
        
        return std::max(std::max(facet0, facet1), facet2);
    }
}

float trans_table::partial_facet_score(const board_t& board) const {
    float cube1 = cube_score(board >> 32);
    float cube2 = cube_score(CUBE_MASK & board);
    return std::max(cube1, cube2);
}

float trans_table::facet_score(const board_t& board) const {
    if (ALL_CUBES) {
        return max4(partial_facet_score(board),
        partial_facet_score(swap_0_1(board)),
        partial_facet_score(swap_0_2(board)),
        partial_facet_score(swap_0_3(board)));
    } else {
        return partial_facet_score(board);
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
    if (REORGANIZE_BOARD) {
        return reorganized_heuristic(reorganize(board, 0));
    } else {
        return facet_score(board) + partial_row_heuristic(board);
    }
}

float trans_table::reorganized_heuristic(const board_t& board) const {
    return _aug_partial_square_row[ROW_MASK & (board >> 48)]
    + _partial_square_row[ROW_MASK & (board >> 32)]
    - _aug_row_mon_vals[ROW_MASK & (board >> 48)]
    - _aug_row_mon_vals[ROW_MASK & (board >> 32)]
    + secondary_cube_heuristic(board);
}

float trans_table::secondary_cube_heuristic(const board_t& board) const {
    board_t board0 = board;
    board_t board1 = h_board_1(board);
    board_t board2 = h_board_3(board);
    
    float score0 = _partial_heuristic[ROW_MASK & board0] + _partial_heuristic[ROW_MASK & (board0 >> 16)];
    float score1 = _partial_heuristic[ROW_MASK & board1] + _partial_heuristic[ROW_MASK & (board1 >> 16)];
    float score2 = _partial_heuristic[ROW_MASK & board2] + _partial_heuristic[ROW_MASK & (board2 >> 16)];
    
    return 6 * std::max(std::max(score0, score1), score2);
}

float trans_table::heuristic(const board_t& board) const {
    return non_terminal_heuristic(board) - LOSS_PENALTY * is_terminal(board);
}

// move node in expectimax
float trans_table::move_node(const board_t& board, const int& depth, const float& prob, cached_emax_states_t& cached_emax_values, const float& min_prob){
    ++b_eval_count;
    float res = -INFINITY;

    // pick move with greatest expected utility
    int idx = 0;
    u_int16_t move_mask = _valid_move_mask(board);

    // if there are no valid moves, return heuristic
    if (move_mask == 0){
        return heuristic(board);
    }
    
    // iterates over valid move set
    for (int i = 0; move_mask; ++i, move_mask >>= 1) {
        if (move_mask & 1) {
            res = std::max(res, expectation_node(_shift_board(board, DIRECTIONS[i]), depth, prob, cached_emax_values, min_prob));
        }
    }
    
    return res;
}

// expectation node in expectimax
float trans_table::expectation_node(const board_t& board, const int& depth, const float& prob, cached_emax_states_t& cached_emax_values, const float& min_prob){
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
        
        // iterates over empty tiles
        for (board_t randomSetBit = 1; free_tiles; free_tiles >>= 4, randomSetBit <<= 4){
            
            if (free_tiles & 1){
                
                // places 2 in free tile
                res += 0.9 * move_node(board | randomSetBit, depth-1, 0.9 * factor, cached_emax_values, min_prob);
                
                // places 4 in free tile
                res += 0.1 * move_node(board | (randomSetBit << 1), depth-1, 0.1 * factor, cached_emax_values, min_prob);
                
            }
        }
        
        res /= n_empty_tiles;
        
        cached_emax_values[depth][board] = {res, min_prob};
        return res;
    }
}

// entry node  used in multithreaded expectimax
float trans_table::entry_node(const board_t& board, const int& depth, const float& prob, const float& min_prob){
    cached_emax_states_t cached_emax_values = cached_emax_states_t(MAX_DEPTH);
    return expectation_node(board, depth, 1.0, cached_emax_values, min_prob);
}

DIRECTION trans_table::expectimax(const Board& board, const int& depth, const float& min_prob){
    
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

    if (MULTITHREADED) {
        
        std::unordered_map<DIRECTION, std::future<float>> parallel_move_scores;
        
        for (auto move : moves){
    
            std::future<float> fut = std::async(
                std::launch::async, &trans_table::entry_node, this, _shift_board(board.board, move), depth, 1.0, min_prob);
            
            parallel_move_scores[move] = std::move(fut);
        }
        
        for (auto move : moves){
            move_scores.push_back({move, parallel_move_scores[move].get()});
        }

    } else {
        
        cached_emax_states_t cached_emax_values = cached_emax_states_t(MAX_DEPTH);
        
        for (auto move : moves){
            
            float next_score = expectation_node(_shift_board(board.board, move), depth, 1.0, cached_emax_values, min_prob);
            
            move_scores.push_back({move, next_score});
        }
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

long long trans_table::mcts_score(const Board& board, const DIRECTION& move, const size_t& n_sims){
    long long tmp_score = 0;
    
    for (int i = 0; i < n_sims; ++i){
        Board tmp_board = board;
        tmp_board.move(move);
        
        while (!is_terminal(tmp_board.board)){
            tmp_board.move(tmp_board.random_move());
        }
        tmp_score += tmp_board.score();
    }
    
    return tmp_score;
}

DIRECTION trans_table::mcts(const Board& board, const size_t& n_sims){
    std::vector<DIRECTION> moves = board.valid_moves();
    assert (moves.size() > 0);
    
    DIRECTION res = moves[0];
    
    if (MULTITHREADED) {
        
        robin_hood::unordered_map<DIRECTION, std::future<long long>> parallel_move_scores;
        
        for (auto move : moves){
    
            std::future<long long> fut = std::async(
                std::launch::async, &trans_table::mcts_score, this, board,  move, n_sims);
            
            parallel_move_scores[move] = std::move(fut);
        }
        
        long long best_score = 0;
        
        for (auto move : moves){
            long long tmp_score = parallel_move_scores[move].get();
            
            if (tmp_score > best_score){
                best_score = tmp_score;
                res = move;
            }
        }
        
        return res;
        
    } else {
        long long best_score = 0;
        
        for (auto move : moves){
            
            long long tmp_score = mcts_score(board, move, n_sims);
            
            if (tmp_score > best_score){
                best_score = tmp_score;
                res = move;
            }
        }
        
        return res;
    }
}
