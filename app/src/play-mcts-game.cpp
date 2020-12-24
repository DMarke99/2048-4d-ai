#include "game.hpp"
const bool SHOW_ANALYTICS = true;

int main(int argc, char *argv[]) {
    assert ((argc == 1) | (argc == 2));
    switch (argc){
        case 2: display_mcts_game(atoi(argv[1]), SHOW_ANALYTICS); break;
        default: display_mcts_game(2000, SHOW_ANALYTICS); break;
    }
    return 0;
}
