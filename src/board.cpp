#include "board.hpp"

#if defined( __builtin_popcountll)
int popcount(const u_int64_t& x){
    return __builtin_popcountll(x);
}
#else
int popcount(const u_int64_t& x){
    return _popcount(x);
}
#endif

// declaring all global constants from header file
bool initialized = false;
const DIRECTION DIRECTIONS[8] = {L, LL, R, RR, U, UU, D, DD};
const board_t ROW_MASK = 0x000000000000ffff;
const board_t COL_MASK = 0x000f000f000f000f;
board_t l_move_table[65536];
board_t ll_move_table[65536];
board_t r_move_table[65536];
board_t rr_move_table[65536];
board_t u_move_table[65536];
board_t uu_move_table[65536];
board_t d_move_table[65536];
board_t dd_move_table[65536];
board_t row_to_col[65536];
int row_val[65536];
float row_mon_vals[65536];
float row_pow_val[65536];
float row_edge_val[65536];
int n_row_merges[65536];
bool row_is_terminal[65536];

size_t MAX_DEPTH = 16;
float MONOTONICITY_BASE = 3;
float BOARD_VALUE_BASE = 2.75;
int LOSS_PENALTY = 1000000;

int _popcount(u_int64_t x){
    int i = 0;
    for (; x; ++i){x &= (x - 1);}
    return i;
}

u_int64_t selectBit(u_int64_t v, u_int32_t r) {
    // Source: https://graphics.stanford.edu/~seander/bithacks.html
    // v - Input:  value to find position with rank r.
    // r - Input: bit's desired rank [1-64].
    u_int32_t s;      // Output: Resulting position of bit with rank r [1-64]
    u_int64_t a, b, c, d; // Intermediate temporaries for bit count.
    u_int32_t t;      // Bit count temporary.

    // Do a normal parallel bit count for a 64-bit integer,
    // but store all intermediate steps.
    a =  v - ((v >> 1) & ~0UL/3);
    b = (a & ~0UL/5) + ((a >> 2) & ~0UL/5);
    c = (b + (b >> 4)) & ~0UL/0x11;
    d = (c + (c >> 8)) & ~0UL/0x101;
    t = (d >> 32) + (d >> 48);
    
    // Now do branchless select!
    s  = 64;
    s -= ((t - r) & 256) >> 3; r -= (t & ((t - r) >> 8));
    t  = (d >> (s - 16)) & 0xff;
    s -= ((t - r) & 256) >> 4; r -= (t & ((t - r) >> 8));
    t  = (c >> (s - 8)) & 0xf;
    s -= ((t - r) & 256) >> 5; r -= (t & ((t - r) >> 8));
    t  = (b >> (s - 4)) & 0x7;
    s -= ((t - r) & 256) >> 6; r -= (t & ((t - r) >> 8));
    t  = (a >> (s - 2)) & 0x3;
    s -= ((t - r) & 256) >> 7; r -= (t & ((t - r) >> 8));
    t  = (v >> (s - 1)) & 0x1;
    s -= ((t - r) & 256) >> 8;
    return 64-s;
}


board_t arr_to_row_transition(std::vector<board_t> arr) {
    return (arr[0] << 12) + (arr[1] << 8) + (arr[2] << 4) + arr[3];
}

board_t arr_to_col_transition(std::vector<board_t> arr) {
    return (arr[0] << 48) + (arr[1] << 32) + (arr[2] << 16) + arr[3];
}

int zero_count(std::vector<board_t> arr) {
    int idx = 0;
    for (auto x : arr){if (x == 0) ++idx;}
    return idx;
}

std::vector<board_t> roll_array(std::vector<board_t> arr){
    board_t tmp = arr[0];
    arr.erase (arr.begin());
    arr.emplace_back(tmp);
    return arr;
}

// turns 0xA000B000C000D into 0xABCD
board_t col_to_row(const board_t& col) {
    return ((col >> 36) | (col >> 24) | (col >> 12) | col ) & 0xffff;
}

// turns 0x0123456789ABCDEF into 048C159D26AE37BF
board_t transpose(const board_t& board){
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
board_t swap_0_1(const board_t& board){
    board_t a1 = board & 0xffff00000000ffff;
    board_t a2 = board & 0x0000ffff00000000;
    board_t a3 = board & 0x00000000ffff0000;
    return a1 | (a2 >> 16) | (a3 << 16);
}

// swaps 0th and 2nd indexes
board_t swap_0_2(const board_t& board){
    board_t a1 = board & 0xff00ff0000ff00ff;
    board_t a2 = board & 0x00ff00ff00000000;
    board_t a3 = board & 0x00000000ff00ff00;
    return a1 | (a2 >> 24) | (a3 << 24);
}

// swaps 0th and 3rd indexes
board_t swap_0_3(const board_t& board){
    board_t a1 = board & 0xf0f0f0f00f0f0f0f;
    board_t a2 = board & 0x0f0f0f0f00000000;
    board_t a3 = board & 0x00000000f0f0f0f0;
    return a1 | (a2 >> 28) | (a3 << 28);
}

// swaps 1st and 2nd indexes
board_t swap_1_2(const board_t& board){
    board_t a1 = board & 0xff0000ffff0000ff;
    board_t a2 = board & 0x00ff000000ff0000;
    board_t a3 = board & 0x0000ff000000ff00;
    return a1 | (a2 >> 8) | (a3 << 8);
}

// swaps 1st and 3rd indexes
board_t swap_1_3(const board_t& board){
    board_t a1 = board & 0xf0f00f0ff0f00f0f;
    board_t a2 = board & 0x0f0f00000f0f0000;
    board_t a3 = board & 0x0000f0f00000f0f0;
    return a1 | (a2 >> 12) | (a3 << 12);
}

// swaps 2nd and 3rd indexes
board_t swap_2_3(const board_t& board){
    board_t a1 = board & 0xf00ff00ff00ff00f;
    board_t a2 = board & 0x0f000f000f000f00;
    board_t a3 = board & 0x00f000f000f000f0;
    return a1 | (a2 >> 4) | (a3 << 4);
}

//performs L move on one row
std::vector<board_t> reduce_l(std::vector<board_t> arr){
    for (int i = 0; i < 4; i+=2){
        if (arr[i] == 0){
            std::swap(arr[i], arr[i+1]);
            
        } else if (arr[i+1] == 0) {
            
        } else if (arr[i] == arr[i+1]) {
            if (arr[i] < 15) arr[i] += 1; // imagines that 32768 + 32768 = 32768
            arr[i+1] = 0;
        }
    }
    
    return arr;
}

//performs LL move on one row
std::vector<board_t> reduce_ll(std::vector<board_t> arr){
    for (int i = 0; i < 2; ++i){
        if (arr[i] == 0){
            std::swap(arr[i], arr[i+2]);
        } else if (arr[i+2] == 0) {
            
        } else if (arr[i] == arr[i+2]) {
            if (arr[i] < 15) arr[i] += 1; // imagines that 32768 + 32768 = 32768
            arr[i+2] = 0;
        }
    }
    
    return arr;
}

//performs R move on one row
std::vector<board_t> reduce_r(std::vector<board_t> arr){
    std::reverse(arr.begin(), arr.end());
    std::vector<board_t> res = reduce_l(arr);
    std::reverse(res.begin(), res.end());
    return res;
}

//performs RR move on one row
std::vector<board_t> reduce_rr(std::vector<board_t> arr){
    std::reverse(arr.begin(), arr.end());
    std::vector<board_t> res = reduce_ll(arr);
    std::reverse(res.begin(), res.end());
    return res;
}

bool is_terminal_row(std::vector<board_t> arr){
    for (auto i : arr) if (i == 0) return false;
    if (arr[0] == arr[1]) return false;
    if (arr[2] == arr[3]) return false;
    if (arr[0] == arr[2]) return false;
    if (arr[1] == arr[3]) return false;
    return true;
}


int loc_val(const board_t& val){
    return (int) (val << val);
}

float loc_pow_val(float val){
    return pow(BOARD_VALUE_BASE, val);
}

// score that is 0 if and only if there is a monotone loop in the square face
// incentivises lining pieces up in order on square faces
float row_mon_value(std::vector<board_t> arr){
    float res = 4 * std::max(pow(MONOTONICITY_BASE, 16), 1.0); //max possible value
    
    for (int k = 0 ; k < 4; ++k){
        float mon_l = 0;
        float mon_r = 0;
        
        std::swap(arr[2], arr[3]);
        
        for (int i = 0; i < 3; ++i){
            float l = arr[i];
            float r = arr[i+1];
            
            if (l > r) mon_l += (pow(MONOTONICITY_BASE, l) - pow(MONOTONICITY_BASE, r));
            else mon_r += (pow(MONOTONICITY_BASE, r) - pow(MONOTONICITY_BASE, l));
        }
        std::swap(arr[2], arr[3]);
        
        res = std::min({res, mon_l, mon_r});
        
        board_t tmp = arr[0];
        arr.erase (arr.begin());
        arr.emplace_back(tmp);
    
    }
    return res;
}

// determines the maximum number of guaranteed merges in a square face
float row_merge_score(std::vector<board_t> arr){
    float res = 0;
    float tmp;
    std::vector<board_t> tmp_arr;
    
    auto fs = {&reduce_l, &reduce_ll, &reduce_r, &reduce_rr};
    for (auto f : fs){
        tmp_arr = f(arr);
        
        float n_merges = zero_count(tmp_arr) - zero_count(arr);
 
        if ((n_merges > 0) && ((tmp = n_merges + row_merge_score(tmp_arr)) > res)){
            res = tmp;
        }
    }
    return res;
}

// the highest value edge in a square
float edge_pow_value(std::vector<board_t> arr){
    return std::max({
        loc_pow_val(arr[0]) + loc_pow_val(arr[1]),
        loc_pow_val(arr[2]) + loc_pow_val(arr[3]),
        loc_pow_val(arr[0]) + loc_pow_val(arr[2]),
        loc_pow_val(arr[1]) + loc_pow_val(arr[3]),
    });
}

void init_tables(){
    if (!initialized){
        std::vector<board_t> arr;
        board_t row, col;
        
        for (board_t x0 = 0; x0 < 16; ++x0){
            for (board_t x1 = 0; x1 < 16; ++x1){
                for (board_t x2 = 0; x2 < 16; ++x2){
                    for (board_t x3 = 0; x3 < 16; ++x3){
                        arr = {x0, x1, x2, x3};
                        row = arr_to_row_transition(arr);
                        col = arr_to_col_transition(arr);
                        
                        // metadata for board transitions
                        l_move_table[row] = arr_to_row_transition(reduce_l(arr));
                        ll_move_table[row] = arr_to_row_transition(reduce_ll(arr));
                        r_move_table[row] = arr_to_row_transition(reduce_r(arr));
                        rr_move_table[row] = arr_to_row_transition(reduce_rr(arr));
                        u_move_table[row] = arr_to_col_transition(reduce_l(arr));
                        uu_move_table[row] = arr_to_col_transition(reduce_ll(arr));
                        d_move_table[row] = arr_to_col_transition(reduce_r(arr));
                        dd_move_table[row] = arr_to_col_transition(reduce_rr(arr));
                        row_to_col[row] = col;
                        
                        // metadata for board scoring
                        row_val[row] = loc_val(x0) + loc_val(x1) + loc_val(x2) + loc_val(x3);
                        row_pow_val[row] = loc_pow_val(x3) + loc_pow_val(x2) + loc_pow_val(x1) + loc_pow_val(x0);
                        row_edge_val[row] = edge_pow_value(arr);
                        row_mon_vals[row] = row_mon_value(arr);
                        n_row_merges[row] = row_merge_score(arr);
                        row_is_terminal[row] = is_terminal_row(arr);
                    }
                }
            }
        }
        
        initialized = true;
    }
}

bool rows_are_terminal(const board_t& board){
    return row_is_terminal[ROW_MASK & board] &&
    row_is_terminal[ROW_MASK & (board >> 16)] &&
    row_is_terminal[ROW_MASK & (board >> 32)] &&
    row_is_terminal[ROW_MASK & (board >> 48)];
}

bool is_terminal(const board_t& board){
    return rows_are_terminal(board) && rows_are_terminal(transpose(board));
}

// finds blank tiles
board_t is_blank(const board_t& board){
    board_t blanks = ~board;
    
    blanks = blanks & (blanks >> 1);
    blanks = blanks & (blanks >> 2);
    
    return blanks & 0x1111'1111'1111'1111;
}

// current score including spawned tiles
int board_score(const board_t& board){
    return row_val[ROW_MASK & board] +
    row_val[ROW_MASK & (board >> 16)] +
    row_val[ROW_MASK & (board >> 32)] +
    row_val[ROW_MASK & (board >> 48)];
}

size_t _get(const board_t& board, const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3){
    assert ((x0 >= 0) && (x0 < 2));
    assert ((x1 >= 0) && (x1 < 2));
    assert ((x2 >= 0) && (x2 < 2));
    assert ((x3 >= 0) && (x3 < 2));
    
    size_t idx = 15 - (8 * x0 + 4 * x1 + 2 * x2 + x3);
    size_t exponent = (board >> (4 * idx)) & 0xf;
    return (exponent == 0) ? 0 : (1 << exponent);
}

board_t _set(const board_t& board, const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3, size_t val){
    assert ((popcount(val) <= 1) && (val <= 32768)); // val must be a power of two  or zero
    assert ((x0 >= 0) && (x0 < 2));
    assert ((x1 >= 0) && (x1 < 2));
    assert ((x2 >= 0) && (x2 < 2));
    assert ((x3 >= 0) && (x3 < 2));
    
    if (val == 0) return board;
    board_t res = board;
    
    size_t idx = 15 - (8 * x0 + 4 * x1 + 2 * x2 + x3);
    size_t log2 = 0;
    
    for (log2=0; val!=1; val>>=1,++log2);
    
    return (res & (~(0xf << (4 * idx)))) | (log2 << 4 * idx);
}

size_t Board::get(const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3){
    return _get(board, x0, x1, x2, x3);
}

void Board::set(const size_t& x0, const size_t& x1, const size_t& x2, const size_t& x3, const size_t& val){
    board = _set(board, x0, x1, x2, x3, val);
}

board_t move_l(const board_t& board){
    board_t board0 = l_move_table[ROW_MASK & board];
    board_t board1 = l_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = l_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = l_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

board_t move_ll(const board_t& board){
    board_t board0 = ll_move_table[ROW_MASK & board];
    board_t board1 = ll_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = ll_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = ll_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

board_t move_r(const board_t& board){
    board_t board0 = r_move_table[ROW_MASK & board];
    board_t board1 = r_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = r_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = r_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

board_t move_rr(const board_t& board){
    board_t board0 = rr_move_table[ROW_MASK & board];
    board_t board1 = rr_move_table[ROW_MASK & (board >> 16)] << 16;
    board_t board2 = rr_move_table[ROW_MASK & (board >> 32)] << 32;
    board_t board3 = rr_move_table[ROW_MASK & (board >> 48)] << 48;
    return board0 | board1 | board2 | board3;
}

board_t move_u(const board_t& board){
    board_t board0 = u_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = u_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = u_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = u_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

board_t move_uu(const board_t& board){
    board_t board0 = uu_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = uu_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = uu_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = uu_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

board_t move_d(const board_t& board){
    board_t board0 = d_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = d_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = d_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = d_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

board_t move_dd(const board_t& board){
    board_t board0 = dd_move_table[col_to_row(COL_MASK & board)];
    board_t board1 = dd_move_table[col_to_row(COL_MASK & (board >> 4))] << 4;
    board_t board2 = dd_move_table[col_to_row(COL_MASK & (board >> 8))] << 8;
    board_t board3 = dd_move_table[col_to_row(COL_MASK & (board >> 12))] << 12;
    return board0 | board1 | board2 | board3;
}

board_t _shift_board(const board_t& board, const DIRECTION& d) {
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

int Board::score() const {
    return board_score(board) - penalty;
}

size_t _rank(const board_t& board) {
    size_t res = 0;
    board_t tmp = board;
    while (tmp){
        res = std::max(res, (size_t) (tmp & 0xf));
        tmp >>= 4;
    }
    return res;
}

size_t Board::rank() const {
    if (max_tile_exceeded) return 16;
    return _rank(board);
}

size_t _count(const board_t& board, const size_t& rank) {
    size_t res = 0;
    board_t tmp = board;
    while (tmp){
        res += (rank == (tmp & 0xf));
        tmp >>= 4;
    }
    return res;
}

size_t Board::count(const size_t& rank) const {
    return _count(board, rank);
}

board_t Board::shift_board(const DIRECTION& d) {
    board = _shift_board(board, d);
    return board;
}

std::vector<DIRECTION> _valid_moves(const board_t& board){
    std::vector<DIRECTION> res = {};
    for (DIRECTION d : DIRECTIONS){
        if (_shift_board(board, d) != board) res.emplace_back(d);
    }
    return res;
}

u_int16_t _valid_move_mask(const board_t& board) {
    u_int16_t res = 0;
    for (DIRECTION d : DIRECTIONS){
        if (_shift_board(board, d) != board) res = res | (1 << d);
    }
    return res;
}

std::vector<DIRECTION> Board::valid_moves() const {
    return _valid_moves(board);
}

u_int16_t Board::valid_move_mask() const {
    return _valid_move_mask(board);
}

board_t Board::generate_piece(){
    
    // gets blank tiles
    board_t pos = is_blank(board);
    if (pos == 0) return false;
    
    // gets random free location
    uint32_t n_empty_tiles = popcount(pos);
    board_t randomSetBitIndex = 63 - selectBit(pos, rand() % n_empty_tiles + 1);
    board_t randomSetBit = 1;
    
    // determines random piece
    bool spawn_four = (rand() % 10) ? 0 : 1;
    penalty += spawn_four ? 4 : 2;
    randomSetBit <<= (randomSetBitIndex + spawn_four);
    
    // places random piece
    board = board | randomSetBit;
    return board;
}

board_t Board::move(const DIRECTION& d){
    int correction = - (int) count(15);
    
    // only places piece is board state is changed
    if (board != shift_board(d)){
        board = generate_piece();
        correction += count(15);
        
        // determines whether 65536 would have spawned
        if (correction < 0){
            penalty -= 557056;
            max_tile_exceeded = true;
        }
    }
    return board;
}

DIRECTION Board::random_move() const {
    std::vector<DIRECTION> moveset = valid_moves();
    return moveset[rand() % moveset.size()];
}

std::ostream& operator<<(std::ostream& os, const Board& B){
    int val;
    
    os << "[Score: " << std::dec << B.score() << "]" << std::endl;
    os << "[Rank: " << (1 << B.rank()) << "]" << std::endl;
    
    os << "┌─────┬─────┬─────┬─────┐" << std::endl;
    
    for (int i = 3; i >= 0; --i){
        for (int j = 0; j < 4; ++j){
            val = (B.board >> (16 * i + 4 * (3-j))) & 0xf;
            
            if (val != 0) os << "|" << std::setw(5) << std::dec << (1 << val);
            else os << "|" << std::setw(5) << " ";
        }
        
        os << "|" << std::endl;
        if (i != 0) os << "├─────┼─────┼─────┼─────┤" << std::endl;
    }
    os << "└─────┴─────┴─────┴─────┘" << std::endl;
    
    return os;
}
