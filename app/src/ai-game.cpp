#include "game.hpp"

int main() {
    
    display_ai_game(4, 0.01);
    //test_params(4, 0.025, 200);
    
    //std::cout << "Hardest Transition D=4 p=0.005" << std::endl;
    //std::cout << std::setprecision(4) << test_transition(4, 0.005, 0xFECDBA8900000000, 16, 2, 200) << std::endl;
    
    //std::cout << "2nd Hardest Transition D=4 p=0.005" << std::endl;
    //std::cout << std::setprecision(4) << test_transition(4, 0.005, 0xEDBCA978, 15, 2, 200) << std::endl;
    return 0;

}
