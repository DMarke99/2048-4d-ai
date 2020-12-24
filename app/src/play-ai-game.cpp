#include "game.hpp"
const bool SHOW_ANALYTICS = true;

int main(int argc, char *argv[]) {
    assert ((argc == 1) | (argc == 3));
    switch (argc){
        case 3: display_ai_game(atoi(argv[1]), atof(argv[2]), SHOW_ANALYTICS); break;
        default: display_ai_game(6, 0.01, SHOW_ANALYTICS); break;
    }
    return 0;
}
