#include "game.hpp"

const float FPS_WINDOW = 5.0;

void display_ai_game(int depth, float min_prob){
    srand((u_int32_t) time(NULL));
    init_tables();

    trans_table T = trans_table({
        800, //merge weight
        600, //blank weight
        20, //cubic face weight
        15, //square face weight
        5,  //edge weight
        1   //monotone curl weight
    });
    
    // generates board
    Board B = Board();
    std::queue<std::chrono::time_point<std::chrono::system_clock>> move_times;
    auto start = std::chrono::system_clock::now();
    move_times.push(start);
    
    B.generate_piece();
    B.generate_piece();
    
    // outputs initial board
    std::cout << "      [[ 2048-4D ]]      " << std::endl;
    std::cout << B << std::endl;
    std::cout << "[Moves/s:            0.0]" << std::endl;
    
    // plays game
    while (B.valid_moves().size()){
        printf("\e[14A");
        
        // calculates optimal move
        DIRECTION best_move = T.expectimax(B, depth, min_prob);
        
        // performs best move
        B.move(best_move);
        
        // reports statistics
        move_times.push(std::chrono::system_clock::now());
        while (std::chrono::duration_cast<std::chrono::milliseconds>(move_times.back() - move_times.front()).count() > FPS_WINDOW * 1000) move_times.pop();
        
        auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(move_times.back() - move_times.front()).count();
        float moves_per_second = (1000.0 * (move_times.size() - 1)) / time_elapsed;
        
        // outputs board
        printf("\e[K");
        std::cout << "      [[ 2048-4D ]]      " << std::endl;
        std::cout << B << std::endl;
        std::cout << "[Moves/second:" << std::setw(10) << std::setprecision(5) << moves_per_second << "]" << std::endl;

    }
    std::cout << "Final Score: " << B.score() << std::endl;
}

void test_params(int depth, float min_prob, size_t n_sims){
    srand((u_int32_t) time(NULL));
    init_tables();

    std::stringstream filepath;;
    filepath << "/Users/diamor/Documents/Documents/Programming/C++ Projects/2048-4d-ai/2048-4d-ai-test ";
    filepath << "(D=" << depth;
    filepath << ", P=" << std::setprecision(5) << min_prob << ").txt";
    
    std::ofstream myfile;
    myfile.open(filepath.str(), std::ios::app);
    
    trans_table T = trans_table({
        800, //merge weight
        600, //blank weight
        20, //cubic face weight
        15, //square face weight
        5,  //edge weight
        1   //monotone curl weight
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
