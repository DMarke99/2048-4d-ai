#include "game.hpp"

int main() {
    
    //display_ai_game(4, 0.05);
    //test_params(4, 0.025, 200);
    
    //std::cout << "Hardest Transition 2 steps harder augmented heuristic" << std::endl;
    //std::cout << std::setprecision(4) << test_transition(4, 0.05, 0xFECDBA, 16, {859, 361, 41, 6, 3, 2}, 2, 200, true) << std::endl;
    
    std::cout << "2nd Hardest Transition 2 steps harder augmented heuristic" << std::endl;
    std::cout << std::setprecision(4) << test_transition(4, 0.05, 0xED00BC00A9, 15, {800, 600, 20, 15, 5, 1}, 2, 100, true) << std::endl;
    //{859, 361, 41, 6, 3, 2}
    //{800, 400, 20, 5, 3, 2,}
    //std::cout << "Last transition 2 steps harder augmented heuristic" << std::endl;
    //test_transition_random_params(4, 0.05, 0xFECDBA, 16, 2, 100, 100);
    
    //std::cout << "2nd to last transition 2 steps harder" << std::endl;
    //test_transition_random_params(5, 0.05, 0xEDBCA9, 15, 2, 100, 100);
    return 0;

}
