#include "game.hpp"

const float FPS_WINDOW = 5.0;

const std::vector<float> PARAMS = {
    50.0f, //merge weight
    300.0f, //blank weight
    20.0f, //cubic face weight
    5.0f, //square face weight relative to cube
    3.0f,  //edge weight relative to square
    2.0f,  //monotone curl weight
};

void display_ai_game(int depth, float min_prob, bool show_analytics){
    srand((u_int32_t) time(NULL));
    

    trans_table T(PARAMS);

    // generates board
    Board B = Board();
    B.generate_piece();
    B.generate_piece();
    
    std::queue<std::chrono::time_point<std::chrono::system_clock>> move_times;
    auto start = std::chrono::system_clock::now();
    move_times.push(start);
    
    float count = 0;
    // outputs initial board
    std::cout << "      [[ 2048-4D ]]      " << std::endl;
    std::cout << B << std::endl;
    
    if (show_analytics) {
        std::cout << "      [ Analytics ]      " << std::endl;
        std::cout << "[Moves/s:            0.0]" << std::endl;
        std::cout << "[AvgMoves/s:         0.0]" << std::endl;
        std::cout << "[BoardEval/s:          0]" << std::endl;
    }
    
    // plays game
    while (B.valid_moves().size()){
        if (show_analytics) {
            printf("\e[17A");
        } else {
            printf("\e[13A");
        }
        
        // calculates optimal move
        DIRECTION best_move = T.expectimax(B, depth, min_prob);
        
        // performs best move
        B.move(best_move);
        
        // reports statistics
        auto end = std::chrono::system_clock::now();
        move_times.push(end);
        while (std::chrono::duration_cast<std::chrono::milliseconds>(move_times.back() - move_times.front()).count() > FPS_WINDOW * 1000) move_times.pop();
        
        auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(move_times.back() - move_times.front()).count();
        float moves_per_second = (1000.0 * (move_times.size() - 1)) / time_elapsed;
        
        ++count;
        
        auto total_time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        float avg_moves_per_second = (1000.0 * count) / total_time_elapsed;
        int board_evals_per_second = (1000.0 * T.b_eval_count.load()) / total_time_elapsed;
        
        // outputs board
        printf("\e[K");
        std::cout << "      [[ 2048-4D ]]      " << std::endl;
        std::cout << B << std::endl;
        
        if (show_analytics) {
            std::cout << "      [ Analytics ]      " << std::endl;
            std::cout << "[Moves/s:" << std::setw(15) << std::setprecision(5) << moves_per_second << "]" << std::endl;
            std::cout << "[AvgMoves/s:" << std::setw(12) << std::setprecision(5) << avg_moves_per_second << "]" << std::endl;
            std::cout << "[BoardEval/s:" << std::setw(11) << board_evals_per_second << "]" << std::endl;
        }
    }
    std::cout << "Final Score: " << B.score() << std::endl;
}

void display_mcts_game(int n_sims, bool show_analytics){
    srand((u_int32_t) time(NULL));
    

    trans_table T(PARAMS);

    // generates board
    Board B = Board();
    B.generate_piece();
    B.generate_piece();
    
    std::queue<std::chrono::time_point<std::chrono::system_clock>> move_times;
    auto start = std::chrono::system_clock::now();
    move_times.push(start);
    
    float count = 0;
    // outputs initial board
    std::cout << "      [[ 2048-4D ]]      " << std::endl;
    std::cout << B << std::endl;
    
    if (show_analytics) {
        std::cout << "      [ Analytics ]      " << std::endl;
        std::cout << "[Moves/s:            0.0]" << std::endl;
        std::cout << "[AvgMoves/s:         0.0]" << std::endl;
    }
    
    // plays game
    while (B.valid_moves().size()){
        if (show_analytics) {
            printf("\e[16A");
        } else {
            printf("\e[13A");
        }
        
        // calculates optimal move
        DIRECTION best_move = T.mcts(B.board, n_sims);
        
        // performs best move
        B.move(best_move);
        
        // reports statistics
        auto end = std::chrono::system_clock::now();
        move_times.push(end);
        while (std::chrono::duration_cast<std::chrono::milliseconds>(move_times.back() - move_times.front()).count() > FPS_WINDOW * 1000) move_times.pop();
        
        auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(move_times.back() - move_times.front()).count();
        float moves_per_second = (1000.0 * (move_times.size() - 1)) / time_elapsed;
        
        ++count;
        
        auto total_time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        float avg_moves_per_second = (1000.0 * count) / total_time_elapsed;
        
        // outputs board
        printf("\e[K");
        std::cout << "      [[ 2048-4D ]]      " << std::endl;
        std::cout << B << std::endl;
        
        if (show_analytics) {
            std::cout << "      [ Analytics ]      " << std::endl;
            std::cout << "[Moves/s:" << std::setw(15) << std::setprecision(5) << moves_per_second << "]" << std::endl;
            std::cout << "[AvgMoves/s:" << std::setw(12) << std::setprecision(5) << avg_moves_per_second << "]" << std::endl;
        }
    }
    std::cout << "Final Score: " << B.score() << std::endl;
}

void test_params(int depth, float min_prob, size_t n_sims){
    srand((u_int32_t) time(NULL));
    

    std::stringstream filepath;;
    filepath << "/ADD/FILE/PATH/HERE/2048-4d-ai-test ";
    filepath << "(D=" << depth;
    filepath << ", P=" << std::setprecision(5) << min_prob << ").txt";
    
    std::ofstream myfile;
    myfile.open(filepath.str(), std::ios::app);
    
    trans_table T(PARAMS);
    
    for (int i = 0; i < n_sims; ++i){
        
        // generates board
        Board B = Board();
        B.generate_piece();
        B.generate_piece();
        
        // plays game
        while (B.valid_moves().size()){

            // calculates optimal move
            DIRECTION best_move = T.expectimax(B, depth, min_prob);
            
            // performs best move
            B.move(best_move);
            
        }
        
        myfile << "{score=" << B.score() << ", rank=" << (1 << B.rank()) << "}" << std::endl;
        std::cout << "Final Score: " << B.score() << std::endl;
    }
    
    myfile.close();
}

// estimates the success probability from a start point of reaching a given rank
float test_transition(int depth, float min_prob, board_t initial_pos, size_t terminal_rank, std::vector<float> params, size_t n_gens, size_t n_games, bool verbose){
    srand((u_int32_t) time(NULL));
    
    trans_table T(params);
    
    float success_counter = 0;
    for (int i = 0; i < n_games; ++i){
        // generates board
        Board B = Board(initial_pos);

        for (int j = 0; j < n_gens; ++j) B.generate_piece();
        // plays game
        while ((B.valid_moves().size()) && (B.rank() < terminal_rank)){
        
            // calculates optimal move
            DIRECTION best_move = T.expectimax(B, depth, min_prob);
            
            // performs best move
            B.move(best_move);

        }
        
        if (B.rank() == terminal_rank){
            ++success_counter;
            if (verbose) std::cout << "Success" << std::endl;
        } else {
            if (verbose) std::cout << "Failure" << std::endl;
        }
    }
    
    return success_counter / n_games;
}

// estimates the success probability from a start point of reaching a given rank
void test_transition_random_params(int depth, float min_prob, board_t initial_pos, size_t terminal_rank, size_t n_gens, size_t n_games, size_t n_sims){
    srand((u_int32_t) time(NULL));
    
    for (int trial = 0; trial < n_sims; ++trial){
        
        std::vector<float> params = {
            (float) (rand() % 1000),
            (float) (rand() % 1000),
            (float) (rand() % 50),
            (float) (rand() % 20 + 1) / 4.0f,
            (float) (rand() % 20 + 1) / 4.0f,
            (float) (rand() % 20 + 1) / 4.0f};
        
        float success_rate = test_transition(depth, min_prob, initial_pos, terminal_rank, params, 2, n_games);
        
        std::cout << "Params:" << params << std::endl;
        std::cout << "Success Rate:" << success_rate << std::endl;
        std::cout << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const std::vector<float>& v){
    os << "{";
    for (int i = 0; i < v.size() - 1; ++i) os << v[i] << ", ";
    os << v[v.size()-1] << "}";
    return os;
}
