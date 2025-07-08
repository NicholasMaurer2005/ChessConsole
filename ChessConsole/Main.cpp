
/*
If you are the Cherno reviewing this first of all Thanks, and second I wan't to point out why I think this C++ code is so good.

I use const and constexpr whenever possible. I try to avoid "magic numbers", instead creating a header file called ChessConstants 
to store every reusable number. I tried my best to use the correct data type for every scenario, using std::uint32_t whenever I 
dont need negative numbers, and std::size_t whenever I'm accessing an array or inside a for loop. The entire code runs on the stack. 
Finally, I make use of the mostmodern c++ featurs and try to use the most modern approach.

The only thing wrong with the code is I was very inconsistent with my naming convention. For some reason, I kept switching back
and forth between snake-case and camel-case. I honestly don't know what I was thinking there seems to be no reason why I used
one over the other, but hey it is old code.

I dont know if only using the stack was good or bad because it uses over 2.3 megabytes of stack, but I wanted the code to run as fast as possible,
and most of that data is used up by the pregenerated moves which needs to be accessed millions of times a second. Because of that
I had to give my project more than the default stack reserve, I gave it 4 megabytes.
*/



/*
I was following a tutorial by https ://www.youtube.com/@chessprogramming591, he wrote his in C. I followed along in C++ 
making lots of optimizations and taking advantage of object - oriented programming.The actual chess engine runs on
"bitboards" or std::uint64_t's. All of the board data and move data is stored in std::uint64_t's so that it is fast
and memory - efficient.For example, the board is represented in just twelve std::uint64_t's, one for every piece, where 
each bit represents a piece on the board, picture an 8x8 grid of bits :

8    0 0 0 0 0 0 0 0
7    0 0 0 0 0 0 0 0
6    0 0 0 0 0 0 0 0
5    0 0 0 0 0 0 0 0
4    0 0 0 0 0 0 0 0
3    0 0 0 0 0 0 0 0
2    0 0 0 0 0 0 0 0
1    0 0 0 0 0 0 0 0

A B C D E F G H

0   r n b q k b n r
1   p p p p p p p p
2   0 0 0 0 0 0 0 0
3   0 0 0 0 0 0 0 0
2   0 0 0 0 0 0 0 0
1   0 0 0 0 0 0 0 0
6   P P P P P P P P
7   R K B Q K B K R

A B C D E F G H

The engine also uses pre - generated moves.Instead of calculating where certain pieces can move on the fly, all possible
moves are calculated when the program starts and stored in memory.Slider pieces use a system called "magic numbers",
where the current board occupancy is hashed and used to look up an index for the available slots for that piece to move
to.This allows for very fast move generation.The actual minimax algorithm is basic, evaluating the board based on piece
value and available moves.The more pieces it has and the more space its pieces can see the higher the evaluation.Checkmates
are given either INT_MAX or INT_MIN depending on which side is being checkmated.By default, it looks ahead 8 moves, on my
computer which causes it to think for about 20 seconds per move at the beginning, but it quickly gets faster as there are
fewer possible moves.It can be changed by altering "depth" in the engine.step() call.
*/



/*
HOW TO USE:

Engine is created and given a FEN, but I didn't fully implement FEN notation so instead It can only read the first part of the FEN 
explaining the board positions.For example, the starting position : rnbqkbnr / pppppppp / 8 / 8 / 8 / 8 / PPPPPPPP / RNBQKBNR
You cannot tell the engine which side needs to castle, which side can move, or anything else that a normal FEN tells.I call it a 90 % FEN.

engine.step() is given three values, which side the engine plays, whether to flip the board when displaying, and how many moves
ahead to look.If it takes way too long to generate moves, lower the depth value so it doesn't look so far ahead. 

When running it will display the board, white pieces are uppercase, and black pieces are lowercase.black pawns are represented with an
uppercase X instead of a lowercase p because it is hard to distinguish between P and p when the board is displayed.It also displays the most recent move and some
information about it :
pr - whether it promoted
ca - whether it was a capture move
en - whether the move was a pawn en passant
cas - whether the move was a castle

nodes - how many moves it made and searched,
evaluations - how many board evaluations it made,
prunes - how many branches it "pruned" or stopped searching due to alpha - beta pruning,
mates - how many checkmates it found.

Finally, it displays how long the move took.

to input a normal move you need to give the starting position and the ending position in all lowercase with no spaces.It cannot parse real
chess moves like e4 or bxh6.Some examples of moves it accepts are d2d4, d7d5, c1f4.If you give a move in the wrong format or a move that doesn't exist the 
engine will display "move does not exist".Even though the engine will display an x in between the positions when capturing it cannot
parse that as an input so even when capturing only input the start and end position.To castle, you have four options :
bk - black kings side castle
bq - black queens side castle
wk - white kings side castle
wq - white queens side castle

inside ChessConstants.hpp the constexpr bool USING_PREGENERATED_MAGICS is used to tell the program whether to generate new magic numbers
or use the ones inside PregeneratedMagics.hpp.Generating new magic numbers doesn't take long, especially in release mode but it is 
redundant because there are only 128 numbers.It generates moves every time it runs because that is 2.3 megabytes of data which would be
impractical to store.After all, it can generate that in milliseconds.

Finally, this program is significantly faster in release mode.
*/



/*
this is still a work in progress, I plan to add multithreading, iterative deepening, better move sorting, and a more sophisticated evaluation
algorithm.

right now there are some strange bugs, sometimes the engine will spawn pieces onto the board or make pieces move where they shouldn't 
be allowed to.I have not figured out why yet because it only happens many moves into a game and when I try to recreate it by giving the FEN
of that game I can't seem to. I think there is something wrong with castling because it marks the illegal move as a castle move.
*/



#include "Engine.h"
#include "ChessConstants.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector> // Required for splitting string

// Helper function to split a string by a delimiter
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main()
{
    Engine engine; // Create one engine instance

    // Set output to line buffered if possible, or disable buffering for stdout
    // This helps ensure the Go process receives data promptly.
    std::cout.setf(std::ios::unitbuf);
    // Alternatively, for more immediate flushing:
    // setvbuf(stdout, NULL, _IONBF, 0);


    std::string line;
    while (std::getline(std::cin, line))
    {
        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.empty()) {
            continue;
        }

        std::string command = tokens[0];

        if (command == "QUIT") {
            // std::cerr << "INFO: Quitting." << std::endl; // Optional: log to stderr
            break;
        } else if (command == "NEW_GAME") {
            std::string fen = start_position_fen; // Default FEN
            if (tokens.size() > 1) {
                fen = tokens[1];
            }
            engine.setState(fen);
            std::cout << "FEN: " << engine.getFEN() << std::endl;
            std::cerr << "INFO: NEW_GAME processed. FEN set to: " << fen << std::endl;
        } else if (command == "MAKE_MOVE") {
            if (tokens.size() < 3) {
                std::cout << "ERROR: Not enough arguments for MAKE_MOVE. Expected FEN and MOVE." << std::endl;
                std::cerr << "ERROR: Not enough arguments for MAKE_MOVE. Expected FEN and MOVE." << std::endl;
                continue;
            }
            std::string current_fen = tokens[1];
            std::string move_string = tokens[2];

            engine.setState(current_fen);

            Move parsed_move;
            if (engine.parseMoveString(move_string, parsed_move)) {
                State& current_engine_state = engine.getCurrentState(); // Get current state from engine
                if (engine.makeMove(parsed_move, current_engine_state)) { // Make move on that state
                    current_engine_state.flipSide(); // makeMove doesn't flip side, but game logic implies it should
                    std::cout << "FEN: " << engine.getFEN() << std::endl;
                    std::cerr << "INFO: MAKE_MOVE processed. Move: " << move_string << ". New FEN: " << engine.getFEN() << std::endl;
                } else {
                    std::cout << "ERROR: Invalid move (rejected by makeMove)." << std::endl;
                    std::cerr << "ERROR: Invalid move (rejected by makeMove) for " << move_string << " on FEN " << current_fen << std::endl;
                }
            } else {
                std::cout << "ERROR: Could not parse or validate move string." << std::endl;
                std::cerr << "ERROR: Could not parse or validate move string: " << move_string << " on FEN " << current_fen << std::endl;
            }
        } else if (command == "GET_ENGINE_MOVE") {
            if (tokens.size() < 2) {
                std::cout << "ERROR: Not enough arguments for GET_ENGINE_MOVE. Expected FEN." << std::endl;
                std::cerr << "ERROR: Not enough arguments for GET_ENGINE_MOVE. Expected FEN." << std::endl;
                continue;
            }
            std::string current_fen = tokens[1];
            // Optional: depth from command `tokens[2]`
            std::uint32_t depth = 8; // Default depth

            engine.setState(current_fen);

            // TODO: Determine side to move from FEN or engine state
            // bool engine_is_white = engine.isWhiteToMove();
            bool engine_is_white = true; // Placeholder

            // The existing engine.step() is interactive and complex.
            // We need a streamlined way to just get the best move.
            // This might involve refactoring engine.step() or calling parts of its logic.
            // For now, let's assume iterativeMinimax updates a best move that we can retrieve.
            // engine.iterativeMinimax(engine.m_state); // m_state is private, needs refactor or public getter
            // Move best_engine_move = engine.getBestMoveFinal(); // Need getBestMoveFinal()
            // std::string move_str = engine.moveToString(best_engine_move); // Need moveToString()
            // engine.makeMove(best_engine_move, engine.m_state); // Make the move on engine's internal state

            // std::cout << "MOVE: " << move_str << " FEN: " << engine.getFEN() << std::endl;
            std::cout << "MOVE: e7e5 FEN: " << "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2" << std::endl; // Placeholder
            std::cerr << "INFO: GET_ENGINE_MOVE (placeholder) processed for FEN: " << current_fen << std::endl;

        } else {
            std::cout << "ERROR: Unknown command: " << command << std::endl;
            std::cerr << "ERROR: Unknown command: " << command << std::endl;
        }
    }

    return 0;
}
