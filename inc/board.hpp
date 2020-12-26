#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <future>
#include <initializer_list>
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

extern const DIRECTION DIRECTIONS[8];
extern const board_t ROW_MASK;
extern const board_t COL_MASK;
extern const board_t CUBE_MASK;
extern const std::array<board_t, 65536> l_move_table;
extern const std::array<board_t, 65536> ll_move_table;
extern const std::array<board_t, 65536> r_move_table;
extern const std::array<board_t, 65536> rr_move_table;
extern const std::array<board_t, 65536> u_move_table;
extern const std::array<board_t, 65536> uu_move_table;
extern const std::array<board_t, 65536> d_move_table;
extern const std::array<board_t, 65536> dd_move_table;
extern const std::array<board_t, 65536> row_to_col;
extern const std::array<int, 65536> row_val;
extern const std::array<float, 65536> row_mon_vals;
extern const std::array<float, 65536> row_pow_val;
extern const std::array<float, 65536> row_edge_val;
extern const std::array<int, 65536> n_row_merges;
extern const std::array<bool, 65536> row_is_terminal;

extern const float MONOTONICITY_BASE;
extern const float BOARD_VALUE_BASE;
extern const int LOSS_PENALTY;

int popcount(const u_int64_t& x);
int _popcount(u_int64_t x);

// board metadata related functions
board_t arr_to_row_transition(std::vector<board_t> arr);
board_t arr_to_col_transition(std::vector<board_t> arr);
int zero_count(std::vector<board_t> arr);
std::vector<board_t> roll_array(std::vector<board_t> arr);
std::vector<board_t> reduce_l(std::vector<board_t> arr);
std::vector<board_t> reduce_ll(std::vector<board_t> arr);
std::vector<board_t> reduce_r(std::vector<board_t> arr);
std::vector<board_t> reduce_rr(std::vector<board_t> arr);
bool is_terminal_row(std::vector<board_t> arr);
float row_merge_score(std::vector<board_t> arr);
float row_mon_value(std::vector<board_t> arr);
float edge_pow_value(std::vector<board_t> arr);
int loc_val(const board_t& val);
float loc_pow_val(float val);

// board manipulation and access methods

// turns 0xa000b000c000d into 0xabcd
constexpr board_t col_to_row(const board_t& col){return ((col >> 36) | (col >> 24) | (col >> 12) | col ) & 0xffff;};

// turns 0x0123456789ABCDEF into 048C159D26AE37BF
constexpr board_t transpose(const board_t& board){
    board_t a1 = board & 0xF0F00F0FF0F00F0FULL;
    board_t a2 = board & 0x0000F0F00000F0F0ULL;
    board_t a3 = board & 0x0F0F00000F0F0000ULL;
    board_t a = a1 | (a2 << 12) | (a3 >> 12);
    board_t b1 = a & 0xFF00FF0000FF00FFULL;
    board_t b2 = a & 0x00FF00FF00000000ULL;
    board_t b3 = a & 0x00000000FF00FF00ULL;
    return b1 | (b2 >> 24) | (b3 << 24);
}

// swaps 0th and 1st indexes
constexpr board_t swap_0_1(const board_t& board){
    board_t a1 = board & 0xffff00000000ffff;
    board_t a2 = board & 0x0000ffff00000000;
    board_t a3 = board & 0x00000000ffff0000;
    return a1 | (a2 >> 16) | (a3 << 16);
}

// swaps 0th and 2nd indexes
constexpr board_t swap_0_2(const board_t& board){
    board_t a1 = board & 0xff00ff0000ff00ff;
    board_t a2 = board & 0x00ff00ff00000000;
    board_t a3 = board & 0x00000000ff00ff00;
    return a1 | (a2 >> 24) | (a3 << 24);
}

// swaps 0th and 3rd indexes
constexpr board_t swap_0_3(const board_t& board){
    board_t a1 = board & 0xf0f0f0f00f0f0f0f;
    board_t a2 = board & 0x0f0f0f0f00000000;
    board_t a3 = board & 0x00000000f0f0f0f0;
    return a1 | (a2 >> 28) | (a3 << 28);
}

// swaps 1st and 2nd indexes
constexpr board_t swap_1_2(const board_t& board){
    board_t a1 = board & 0xff0000ffff0000ff;
    board_t a2 = board & 0x00ff000000ff0000;
    board_t a3 = board & 0x0000ff000000ff00;
    return a1 | (a2 >> 8) | (a3 << 8);
}

// swaps 1st and 3rd indexes
constexpr board_t swap_1_3(const board_t& board){
    board_t a1 = board & 0xf0f00f0ff0f00f0f;
    board_t a2 = board & 0x0f0f00000f0f0000;
    board_t a3 = board & 0x0000f0f00000f0f0;
    return a1 | (a2 >> 12) | (a3 << 12);
}

// swaps 2nd and 3rd indexes
constexpr board_t swap_2_3(const board_t& board){
    board_t a1 = board & 0xf00ff00ff00ff00f;
    board_t a2 = board & 0x0f000f000f000f00;
    board_t a3 = board & 0x00f000f000f000f0;
    return a1 | (a2 >> 4) | (a3 << 4);
}

// flips locations on index 0
constexpr board_t flip_0(const board_t& board){
    return (board >> 32) | (board << 32);
}

// flips locations on index 1
constexpr board_t flip_1(const board_t& board){
    return ((board & 0xffff0000ffff0000) >> 16) | ((board & 0x0000ffff0000ffff) << 16);
}

// flips locations on index 0
constexpr board_t flip_2(const board_t& board){
    return ((board & 0xff00ff00ff00ff00) >> 8) | ((board & 0x00ff00ff00ff00ff) << 8);
}

// swaps 2nd and 3rd indexes
constexpr board_t flip_3(const board_t& board){
    return ((board & 0xf0f0f0f0f0f0f0f0) >> 4) | ((board & 0x0f0f0f0f0f0f0f0f) << 4);
}

// swaps 2nd and 3rd indexes
constexpr board_t flip(const board_t& board, const size_t& idx){
    switch (idx){
        case 0:
            return flip_0(board);
        case 1:
            return flip_1(board);
        case 2:
            return flip_2(board);
        case 3:
            return flip_3(board);
        default:
            return board;
    }
}

constexpr size_t _rank(const board_t& board) {
    size_t res = 0;
    board_t tmp = board;
    while (tmp){
        res = std::max(res, (size_t) (tmp & 0xf));
        tmp >>= 4;
    }
    return res;
}

constexpr size_t _count(const board_t& board, const size_t& rank) {
    size_t res = 0;
    board_t tmp = board;
    while (tmp){
        res += (rank == (tmp & 0xf));
        tmp >>= 4;
    }
    return res;
}

// move based methods
constexpr board_t move_l(const board_t& board){
    board_t board0 = l_move_table[ROW_MASK & board];
    board_t board1 = l_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = l_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = l_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

constexpr board_t move_ll(const board_t& board){
    board_t board0 = ll_move_table[ROW_MASK & board];
    board_t board1 = ll_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = ll_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = ll_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

constexpr board_t move_r(const board_t& board){
    board_t board0 = r_move_table[ROW_MASK & board];
    board_t board1 = r_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = r_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = r_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

constexpr board_t move_rr(const board_t& board){
    board_t board0 = rr_move_table[ROW_MASK & board];
    board_t board1 = rr_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = rr_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = rr_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

constexpr board_t move_u(const board_t& board){
    board_t board0 = u_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = u_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = u_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = u_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

constexpr board_t move_uu(const board_t& board){
    board_t board0 = uu_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = uu_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = uu_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = uu_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

constexpr board_t move_d(const board_t& board){
    board_t board0 = d_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = d_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = d_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = d_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

constexpr board_t move_dd(const board_t& board){
    board_t board0 = dd_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = dd_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = dd_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = dd_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

constexpr board_t _shift_board(const board_t& board, const DIRECTION& d) {
    switch(d) {
    case L:
        return move_l(board);
    case LL:
        return move_ll(board);
    case R:
        return move_r(board);
    case RR:
        return move_rr(board);
    case U:
        return move_u(board);
    case UU:
        return move_uu(board);
    case D:
        return move_d(board);
    case DD:
        return move_dd(board);
    default:
        return 0;
    }
}

constexpr u_int16_t _valid_move_mask(const board_t& board) {
    u_int16_t res = 0;
    for (int d = 0; d < 8; ++d){
        if (_shift_board(board, DIRECTIONS[d]) != board) res = res | (1 << d);
    }
    return res;
}

constexpr bool rows_are_terminal(const board_t& board){
    return row_is_terminal[ROW_MASK & board] &&
    row_is_terminal[ROW_MASK & (board >> 16)] &&
    row_is_terminal[ROW_MASK & (board >> 32)] &&
    row_is_terminal[ROW_MASK & (board >> 48)];
}

constexpr bool _is_terminal(const board_t& board){
    return rows_are_terminal(board) && rows_are_terminal(transpose(board));
}

// finds blank tiles
constexpr board_t is_blank(const board_t& board){
    board_t blanks = ~board;
    blanks = blanks & (blanks >> 1);
    blanks = blanks & (blanks >> 2);
    return blanks & 0x1111'1111'1111'1111;
}

// current score including spawned tiles
constexpr int board_score(const board_t& board){
    return row_val[ROW_MASK & board] +
    row_val[ROW_MASK & (board >> 16)] +
    row_val[ROW_MASK & (board >> 32)] +
    row_val[ROW_MASK & (board >> 48)];
}

size_t _get(const board_t& board, const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3);
board_t _set(const board_t& board, const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3, size_t val);
std::vector<DIRECTION> _valid_moves(const board_t& board);

// encapsulation of 2048-4d board
class Board {
public:
    board_t board = 0;
    int penalty = 0;
    bool max_tile_exceeded = false;
    
    Board(const board_t& board = 0) : board(board) {};
    
    size_t get(const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3) const;
    void set(const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3, const size_t& val);
    int score() const;
    size_t rank() const;
    size_t count(const size_t& rank) const;
    bool is_terminal() const;
    
    // move based methods
    board_t shift_board(const DIRECTION& d);
    std::vector<DIRECTION> valid_moves() const;
    u_int16_t valid_move_mask() const;
    board_t generate_piece();
    board_t move(const DIRECTION& d);
    DIRECTION random_move() const;
};

Board generate_game(size_t n_initial_tiles);
std::ostream& operator<<(std::ostream& os, const Board& B);
