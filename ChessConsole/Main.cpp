#include "Engine.h"
#include "ChessConstants.hpp"

int main()
{
	Engine engine{ start_position_fen };
	engine.step(true, false, 8);
}
