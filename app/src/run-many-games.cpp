#include "game.hpp"

int main(int argc, char *argv[]) {
    assert (argc == 5);
    std::stringstream s;
    s << argv[4];
    test_params(atoi(argv[1]), atof(argv[2]), atoi(argv[3]), s);
    return 0;

}
