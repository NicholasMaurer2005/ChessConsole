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
	
	void popMove(const std::size_t move_index);

	bool findMove(const std::size_t source, const std::size_t target, Move& move_out) const;

	bool findCastleMove(const std::size_t source) const;

	void sortMoveList();

	static bool move_compare(const Move a, const Move b);

	void printMoves() const;

	template<MoveType M, Piece P>
	void addMove(const std::size_t source, const std::size_t target)
	{
		if constexpr (M == MoveType::CAPTURE)
		{
			if constexpr (P == Piece::PAWN)
			{
				m_moves.emplace_back(source, target, true, P, 0b010);
			}
			else
			{
				m_moves.emplace_back(source, target, true, P, 0b011);
			}
		}

		if constexpr (M == MoveType::QUIET)
		{
			m_moves.emplace_back(source, target, false, P, 0b111);
		}

		if constexpr (M == MoveType::QUIET_PROMOTE)
		{
			m_moves.emplace_back(source, target, P, false);
		}

		if constexpr (M == MoveType::PROMOTE)
		{
			m_moves.emplace_back(source, target, P, true);
		}

		if constexpr (M == MoveType::ENPASSANT)
		{
			m_moves.emplace_back(source, target);
		}

		if constexpr (M == MoveType::DOUBLE_PAWN)
		{
			m_moves.emplace_back(source, target, true);
		}
	}

	template <Castle C>
	void addCastleMove()
	{
		if constexpr (C == Castle::BK)
		{
			m_moves.emplace_back(g8);
		}

		if constexpr (C == Castle::BQ)
		{
			m_moves.emplace_back(c8);
		}

		if constexpr (C == Castle::WK)
		{
			m_moves.emplace_back(g1);
		}

		if constexpr (C == Castle::WQ)
		{
			m_moves.emplace_back(c1);
		}
	}
};
