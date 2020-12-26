<p align="center">
<img src="https://i.gyazo.com/fa594b3e6b9d7032118af3ef7a9b2ed9.png" alt="Example AI Game 2048-4D" width="65%">
</p>

AI for the game [2048-4D](https://huonw.github.io/2048-4D/). This uses *expectimax optimization*, along with a highly-efficient bitboard representation of the board to search over 10 millions board states per second. Heuristics used include bonuses for empty squares, setting up merges, and bonuses for placing large valued pieces in the same 1-face (edge), 2-face (square) and 3-face (cube) of the hypercube that represents the board. Inspired by [2048-AI](https://github.com/nneonneo/2048-ai) by @nneonneo.

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

to build the project. Any relatively recent C++ compiler should be able to build the output. To run the game with default parameters execute:

```
bin/play-ai-game
```
To run the game with depth parameter `(int) depth` and minimum probability parameter `(float) min_prob` execute:

```
bin/play-ai-game depth min_prob
```
To run `(size_t) n_games` games with depth parameter `(int) depth` and minimum probability parameter `(float) min_prob`, outputting the results to folder `(string) output_folder` execute:

```
bin/run_many_games depth min_prob n_games output_folder
```
For comparison, to run a game with moves determined by Monte Carlo Tree Search with `(int) n_sims` random games per valid move, execute:

```
bin/play-mcts-game n_sims
```

## Performance

After running 100 games with `depth = 6, min_prob = 0.01`, I observed the following performance:

|Tile|Proportion of games <br /> tile achieved|
|:-:|:-:|
|2048|100%|
|4096|98%|
|8192|98%|
|16384|96%|
|32768|87%|
|65536|15%|

I also observed the following statistics in the sample:
- Minimum Score: 36,924
- Median Score: 845,132
- Maximum Score: 2,212,500
