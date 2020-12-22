#pragma once
#include <algorithm>
#include <cassert>
#include <future>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <unordered_map>
#include <vector>

typedef u_int64_t board_t;

enum DIRECTION {
    L, //LEFT
    LL, //DOUBLE_LEFT
    R, //RIGHT
    RR, //DOUBLE_RIGHT
    U, //UP
    UU, //DOUBLE_UP
    D, //DOWN
    DD //DOUBLE_DOWN
};

extern bool initialized;
extern const DIRECTION DIRECTIONS[8];
extern const board_t ROW_MASK;
extern const board_t COL_MASK;
extern board_t l_move_table[65536];
extern board_t ll_move_table[65536];
extern board_t r_move_table[65536];
extern board_t rr_move_table[65536];
extern board_t u_move_table[65536];
extern board_t uu_move_table[65536];
extern board_t d_move_table[65536];
extern board_t dd_move_table[65536];
extern board_t row_to_col[65536];
extern int row_val[65536];
extern float row_mon_vals[65536];
extern float row_pow_val[65536];
extern float row_edge_val[65536];
extern int n_row_merges[65536];
extern bool row_is_terminal[65536];

extern float MONOTONICITY_BASE;
extern float BOARD_VALUE_BASE;
extern int LOSS_PENALTY;

void init_tables();

int popcount(const u_int64_t& x);
int _popcount(u_int64_t x);
u_int64_t selectBit(u_int64_t v, u_int32_t r);

// board metadata related functions
board_t arr_to_row_transition(std::vector<board_t> arr);
board_t arr_to_col_transition(std::vector<board_t> arr);
int zero_count(std::vector<board_t> arr);
std::vector<board_t> roll_array(std::vector<board_t> arr);
board_t col_to_row(const board_t& col);

std::vector<board_t> reduce_l(std::vector<board_t> arr);
std::vector<board_t> reduce_ll(std::vector<board_t> arr);
std::vector<board_t> reduce_r(std::vector<board_t> arr);
std::vector<board_t> reduce_rr(std::vector<board_t> arr);
bool is_terminal_row(std::vector<board_t> arr);
float row_merge_score(std::vector<board_t> arr);
int loc_val(const board_t& val);

bool rows_are_terminal(const board_t& board);
bool is_terminal(const board_t& board);
board_t is_blank(const board_t& board);
int board_score(const board_t& board);

// board manipulation and access methods
board_t transpose(const board_t& board);
board_t swap_0_1(const board_t& board);
board_t swap_0_2(const board_t& board);
board_t swap_0_3(const board_t& board);
board_t swap_1_2(const board_t& board);
board_t swap_1_3(const board_t& board);
board_t swap_2_3(const board_t& board);
size_t _get(const board_t& board, const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3);
board_t _set(const board_t& board, const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3, size_t val);
size_t _rank(const board_t& board);
size_t _count(const board_t& board, const size_t& rank);

// move based methods
board_t move_l(const board_t& board);
board_t move_ll(const board_t& board);
board_t move_r(const board_t& board);
board_t move_rr(const board_t& board);
board_t move_u(const board_t& board);
board_t move_uu(const board_t& board);
board_t move_d(const board_t& board);
board_t move_dd(const board_t& board);
board_t _shift_board(const board_t& board, const DIRECTION& d);
std::vector<DIRECTION> _valid_moves(const board_t& board);
u_int16_t _valid_move_mask(const board_t& board);

// encapsulation of 2048-4d board
class Board {
public:
    board_t board = 0;
    int penalty = 0;
    bool max_tile_exceeded = false;
    
    Board(const board_t& board = 0) : board(board) {};
    
    size_t get(const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3);
    void set(const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3, const size_t& val);
    int score() const;
    size_t rank() const;
    size_t count(const size_t& rank) const;
    
    // move based methods
    board_t shift_board(const DIRECTION& d);
    std::vector<DIRECTION> valid_moves() const;
    u_int16_t valid_move_mask() const;
    board_t generate_piece();
    board_t move(const DIRECTION& d);
    DIRECTION random_move() const;
};

std::ostream& operator<<(std::ostream& os, const Board& B);
