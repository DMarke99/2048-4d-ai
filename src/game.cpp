#include "game.hpp"

const float FPS_WINDOW = 5.0;

void display_ai_game(int depth, float min_prob){
    srand((u_int32_t) time(NULL));
    init_tables();

    trans_table T = trans_table({
        800, //merge weight
        400, //blank weight
        20, //cubic face weight
        15, //square face weight relative to cube
        5,  //edge weight relative to square
        3,  //monotone curl weight
    });

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
    std::cout << "[Moves/s:            0.0]" << std::endl;
    std::cout << "[AvgMoves/s:         0.0]" << std::endl;
    std::cout << "[BoardEval/s:          0]" << std::endl;
    
    // plays game
    while (B.valid_moves().size()){
        printf("\e[16A");
        
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
        int board_evals_per_second = (1000.0 * T.b_eval_count) / total_time_elapsed;
        
        // outputs board
        printf("\e[K");
        std::cout << "      [[ 2048-4D ]]      " << std::endl;
        std::cout << B << std::endl;
        std::cout << "[Moves/s:" << std::setw(15) << std::setprecision(5) << moves_per_second << "]" << std::endl;
        std::cout << "[AvgMoves/s:" << std::setw(12) << std::setprecision(5) << avg_moves_per_second << "]" << std::endl;
        std::cout << "[BoardEval/s:" << std::setw(11) << board_evals_per_second << "]" << std::endl;

    }
    std::cout << "Final Score: " << B.score() << std::endl;
}

void test_params(int depth, float min_prob, size_t n_sims){
    srand((u_int32_t) time(NULL));
    init_tables();

    std::stringstream filepath;;
    filepath << "/Users/diamor/Documents/Documents/Programming/C++ Projects/2048-4d-ai-test ";
    filepath << "(D=" << depth;
    filepath << ", P=" << std::setprecision(5) << min_prob << ").txt";
    
    std::ofstream myfile;
    myfile.open(filepath.str(), std::ios::app);
    
    trans_table T = trans_table({
        800, //merge weight
        400, //blank weight
        20, //cubic face weight
        15, //square face weight relative to cube
        5,  //edge weight relative to square
        3,  //monotone curl weight
    });
    
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
float test_transition(int depth, float min_prob, board_t initial_pos, size_t terminal_rank, size_t n_gens, size_t n_sims){
    srand((u_int32_t) time(NULL));
    init_tables();
    trans_table T = trans_table({800, 400, 20, 15, 5, 1});
    T = trans_table({800, 400, 20, 15, 5, 3});
    
    float success_counter = 0;
    for (int i = 0; i < n_sims; ++i){
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
            std::cout << "Success" << std::endl;
        } else {
            std::cout << "Failure" << std::endl;
        }
    }
    
    return success_counter / n_sims;
}
