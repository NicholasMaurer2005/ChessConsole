
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

int main()
{
	Engine engine{ start_position_fen };
	//Engine engine{ "k7/r7/8/8/8/8/7R/7K"};
	engine.step(true, false, 4); 
	//engine.perft(10);
}
