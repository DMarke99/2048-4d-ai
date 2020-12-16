# pragma once
# include "board.hpp"

//
template <class T>
T max4(const T& x0, const T& x1, const T& x2, const T& x3);

template <class T>
T max6(const T& x0, const T& x1, const T& x2, const T& x3, const T& x4, const T& x5);

// board manipulation methods used only for calculating heuristics
board_t h_board_0(const board_t& board);
board_t h_board_1(const board_t& board);
board_t h_board_2(const board_t& board);
board_t h_board_3(const board_t& board);
board_t h_board_4(const board_t& board);
board_t h_board_5(const board_t& board);

// transposition table used for 2048-4d-ai
class trans_table {
private:
    std::vector<float> params;
    std::vector<std::unordered_map<board_t, float>> cached_expectimax_values;
    float _partial_square_row[65536];
    float _partial_heuristic[65536];
public:
    trans_table(const std::vector<float>& params={800,600,20,15,5,1});
    float partial_square_score(const board_t& board) const;
    float square_score(const board_t& board) const;
    float cube_score(const board_t& board) const;
    float partial_facet_score(const board_t& board) const;
    float facet_score(const board_t& board) const;
    float partial_heuristic(const board_t& board) const;
    float partial_row_heuristic(const board_t& board) const;
    float heuristic(const board_t& board) const;
    float move_node(const board_t& board, const int& depth, const float& prob, const float& min_prob = 1e-6);
    float expectation_node(const board_t& board, const int& depth, const float& prob, const float& min_prob = 1e-6);
    DIRECTION expectimax(const Board& board, const int& depth, const float& min_prob = 1e-6);
};