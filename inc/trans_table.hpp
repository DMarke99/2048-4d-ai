# pragma once
# include "board.hpp"
# include <future>
# include <atomic>
# include <robin_hood.h>

extern const size_t MAX_DEPTH;
extern const bool REORGANIZE_BOARD;
extern const bool FAST_HEURISTIC;
extern const bool ALL_CUBES;
extern const bool MULTITHREADED;

template <class T>
T max4(const T& x0, const T& x1, const T& x2, const T& x3);

template <class T>
T max6(const T& x0, const T& x1, const T& x2, const T& x3, const T& x4, const T& x5);

struct emax_state {
    float val;
    float min_prob;
};

struct move_state {
    DIRECTION move;
    float emax_val;
};

typedef std::vector<robin_hood::unordered_flat_map<board_t, emax_state>> cached_emax_states_t;

// board manipulation methods used only for calculating heuristics
board_t h_board_0(const board_t& board);
board_t h_board_1(const board_t& board);
board_t h_board_2(const board_t& board);
board_t h_board_3(const board_t& board);
board_t h_board_4(const board_t& board);
board_t h_board_5(const board_t& board);

board_t facet(const board_t& board, const int& f_idx);
board_t h_board(const board_t& board, const int& h_idx);
board_t reorganize(const board_t& board, const size_t& level);

// transposition table used for 2048-4d-ai
class trans_table {
private:
    std::vector<float> params;
    float _partial_square_row[65536];
    float _aug_partial_square_row[65536];
    float _partial_heuristic[65536];
    float _aug_row_mon_vals[65536];
    
public:
    std::atomic<u_int64_t> b_eval_count;
    trans_table(const std::vector<float>& params={800,600,20,15,5,0});
    
    // heuristic based ethods
    float cube_score(const board_t& board) const;
    float partial_facet_score(const board_t& board) const;
    float facet_score(const board_t& board) const;
    float partial_heuristic(const board_t& board) const;
    float partial_row_heuristic(const board_t& board) const;
    float non_terminal_heuristic(const board_t& board) const;
    float reorganized_heuristic(const board_t& board) const;
    float secondary_cube_heuristic(const board_t& board) const;
    float heuristic(const board_t& board) const;
    
    // expectimax methods
    float move_node(const board_t& board, const int& depth, const float& prob, cached_emax_states_t& cached_emax_values,  const float& min_prob = 1e-6);
    float expectation_node(const board_t& board, const int& depth, const float& prob, cached_emax_states_t& cached_emax_values,  const float& min_prob = 1e-6);
    float entry_node(const board_t& board, const int& depth, const float& prob, const float& min_prob = 1e-6);
    DIRECTION expectimax(const Board& board, const int& depth, const float& min_prob = 1e-6);
    
    // monte carlo tree search
    long long mcts_score(const Board& board, const DIRECTION& move, const size_t& n_sims);
    DIRECTION mcts(const Board& board, const size_t& n_sims);
};
