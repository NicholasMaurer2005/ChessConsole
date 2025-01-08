#pragma once

#include "ChessConstants.hpp"
#include "BitBoard.h"
#include "PreGen.h"
#include "State.h"
#include "string"
#include <cstddef>
#include "Move.h"
#include "MoveList.h"

class MoveGen
{
private:
	PreGen m_preGen;

public:
	MoveGen();

	void generateMoves(const State& state, MoveList& moveList);

	//color represents defending side
	bool isSquareAttacked(const State& state, const std::size_t square, const Color side) const;

	BitBoard getBishopAttack(const std::size_t square, const BitBoard occupancy) const;

	BitBoard getRookAttack(const std::size_t square, const BitBoard occupancy) const;

	template<Color C>
	BitBoard getPawnAttack(const std::size_t square) const;

	BitBoard getKnightAttack(const std::size_t square) const;

	BitBoard getKingAttack(const std::size_t square) const;

	BitBoard getPieceAttack(const std::size_t P, std::size_t square, const State& state);
};