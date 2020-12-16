AI for the game [2048-4D](https://huonw.github.io/2048-4D/). This uses *expectimax optimization*, along with a highly-efficient bitboard representation of the board to search millions of moves per second. Heuristics used include bonuses for empty squares, setting up merges, and bonuses for placing large valued pieces in the same 1-face (edge), 2-face (square) and 3-face (cube) of the hypercube that represents the board. Inspired by [2048-AI](https://github.com/nneonneo/2048-ai) by @nneonneo.

## Requirements

```
- A C++ 14 Compiler
- CMake 3.17.3+ (probably works on some earlier versions of CMake 3.x)
- Terminal which supports ANSI Escape Codes for displaying board)
```

## Building
### Unix/Linux/OS X

In a terminal, jump to the project root directory and execute:

```
cmake -S . -B build
cmake --build build
```

to build the project. Any relatively recent C++ compiler should be able to build the output. To run the game execute:

```
bin/2048-ai-game
```
