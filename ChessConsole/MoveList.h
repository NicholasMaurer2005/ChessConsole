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
	void addMove(const std::size_t source, const std::size_t target, const Piece captured_piece)
	{
		if constexpr (M == MoveType::CAPTURE) //TODO: restructure
		{
			if constexpr (P == Piece::PAWN)
			{
				//pawn capture
				m_moves.emplace_back(source, target, Piece::PAWN, captured_piece);
			}
			else
			{
				//regular capture
				m_moves.emplace_back(source, target, P, captured_piece);
			}
		}

		if constexpr (M == MoveType::QUIET)
		{
			//regular quiet
			m_moves.emplace_back(source, target, P, Piece::NO_PIECE);
		}

		if constexpr (M == MoveType::QUIET_PROMOTE)
		{
			//quiet promote
			m_moves.emplace_back(source, target, Piece::PAWN, Piece::NO_PIECE, false);
		}

		if constexpr (M == MoveType::PROMOTE)
		{
			//capture and promote
			m_moves.emplace_back(source, target, Piece::PAWN, captured_piece, true);
		}

		if constexpr (M == MoveType::ENPASSANT)
		{
			//enpassant
			m_moves.emplace_back(source, target, captured_piece);
		}

		if constexpr (M == MoveType::DOUBLE_PAWN)
		{
			m_moves.emplace_back(source, target);
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
