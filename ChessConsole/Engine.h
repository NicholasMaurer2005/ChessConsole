#pragma once

#include "MoveGen.h"
#include "BitBoard.h"
#include "ChessConstants.hpp"
#include <string>
#include <string_view>
#include <cstddef>
#include <algorithm>
#include <chrono>

using namespace std::literals::string_literals;

class Engine
{
private:
	MoveGen m_moveGen;

	State m_state;
	Move m_bestMove;

	std::uint32_t m_depth;

	std::uint32_t m_evaluations;
	std::uint32_t m_nodes;
	std::uint32_t m_prunes;
	std::uint32_t m_mates;
	std::chrono::duration<double> m_seconds;

public:
	Engine();

	Engine(std::string_view fen);

	void setState(std::string_view fen);

	void step(const bool engine_side_white, const bool flip_board, const std::uint32_t depth);

	void printBoard(const bool flipped) const;

	int evaluate(const State& state);

	int minimax(const State& state, const std::uint32_t depth, int alpha, int beta);

	void printAllBoardAttacks(Color C) const;

	bool inputAndParseMove(MoveList& list, Move& move);

	static std::size_t squareToIndex(std::string_view square);

	bool makeMove(const Move move, State& state) const;

	bool kingInCheck(const State& state) const;
};