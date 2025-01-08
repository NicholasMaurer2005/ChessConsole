#pragma once

#include "Move.h"
#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>

class MoveList
{
private:
	std::vector<Move> m_moves;

	std::size_t m_count;

public:
	MoveList();

	const std::vector<Move>& moves() const;

	std::size_t count() const;

	void addMove(const Move move);

	void popMove(const std::size_t move_index);

	bool findMove(const std::size_t source, const std::size_t target, Move& move_out) const;

	bool findCastleMove(const std::size_t source) const;

	void sortMoveList();

	static bool move_compare(const Move a, const Move b);

	void printMoves() const;
};
