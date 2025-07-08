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
	Move m_bestMoveFinal;

	std::uint32_t m_depth;

	bool m_stopSearch;
	int m_timeCheckCount;
	std::chrono::steady_clock::time_point m_searchStartTime;

	std::uint32_t m_depthSearched;
	std::uint32_t m_evaluations;
	std::uint32_t m_nodes;
	std::uint32_t m_prunes;
	std::uint32_t m_mates;
	std::size_t m_moveSource;
	std::chrono::duration<double> m_seconds;

public:
	Engine();

	Engine(std::string_view fen);

	void setState(std::string_view fen);

	void step(const bool engine_side_white, const bool flip_board, const std::uint32_t depth);

	void printBoard(const bool flipped) const;

	int evaluate(const State& state);

	int minimax(const State& state, const std::uint32_t depth, int alpha, int beta);

	void iterativeMinimax(const State& state);

	void printAllBoardAttacks(Color C) const;

	bool inputAndParseMove(MoveList& list, Move& move); // Will likely need to adapt or supplement this

	// New methods for command-based interaction
	std::string getFEN() const;
	bool parseMoveString(const std::string& moveStr, Move& outMove); // Non-interactive move parsing
	std::string moveToString(const Move& move) const;
	Move getBestMoveFinal() const;
	void calculateBestMove(std::uint32_t depth); // Streamlined calculation
	bool isWhiteToMove() const; // To get current turn from state


	static std::size_t squareToIndex(std::string_view square);

	bool makeMove(const Move move, State& state); // Make this public if not already, or provide a wrapper
	// bool makeMove(const Move move); // Simpler public interface if m_state is consistently used

	bool kingInCheck(const State& state) const;

	// Allow Main.cpp to access m_state for iterativeMinimax and makeMove if needed,
	// or provide more public methods to wrap these operations.
	// For now, let's assume we'll add specific public methods.
	friend class Main; // Not ideal, but for quick access to m_state if necessary.
					   // Prefer dedicated methods like `makeMoveOnCurrentState(const Move& move)`
                       // and `calculateBestMoveOnCurrentState(std::uint32_t depth)`
public: // Ensure m_state is accessible or provide getters/wrappers
    State& getCurrentState(); // To allow makeMove and iterativeMinimax to operate on the engine's state
};