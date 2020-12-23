#include "game.hpp"

const std::vector<float> params = {
    800.0f, //merge weight
    300.0f, //blank weight
    20.0f, //cubic face weight
    5.0f, //square face weight relative to cube
    3.0f,  //edge weight relative to square
    2.0f,  //monotone curl weight
};

//Other decent parameter sets
//{859, 361, 41, 6, 3, 2}
//{800, 400, 20, 5, 3, 2}
//{859, 361, 41, 6, 3, 2};
//{800, 450, 40, 7, 5, 5};

int main() {
    
    /*
    std::cout << "3rd Hardest Transition 4 steps harder 10 tiles spawned" << std::endl;
    std::cout << std::setprecision(4) << test_transition(6, 0.02, 0xDCAB, 14, params, 10, 100, true) << std::endl;
    */
    
    std::cout << "2nd Hardest Transition 4 steps harder 10 tiles spawned" << std::endl;
    std::cout << std::setprecision(4) << test_transition(6, 0.01, 0xEDBC, 15, params, 10, 100, true) << std::endl;
    
    std::cout << "Hardest Transition 4 steps harder 10 tiles spawned" << std::endl;
    std::cout << std::setprecision(4) << test_transition(6, 0.01, 0xFECD, 16, params, 10, 100, true) << std::endl;
    

    //std::cout << "Last transition 4 steps harder augmented heuristic" << std::endl;
    //test_transition_random_params(4, 0.05, 0xFECD, 16, 4, 100, 100);
    
    //std::cout << "2nd to last transition 4 steps harder 4 tiles spawned" << std::endl;
    //test_transition_random_params(4, 0.05, 0xEDBC, 15, 4, 100, 100);
    
    //std::cout << "3rd to last transition 4 steps harder" << std::endl;
    //test_transition_random_params(4, 0.05, 0xDCAB, 14, 4, 100, 100);
    return 0;

}
