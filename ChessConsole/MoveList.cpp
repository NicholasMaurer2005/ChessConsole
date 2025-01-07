#include "MoveList.h"


MoveList::MoveList()
	: m_moves(), m_count()
{
	m_moves.reserve(256);
}

const std::vector<Move>& MoveList::moves() const
{
	return m_moves;
}

std::size_t MoveList::count() const
{
	return m_moves.size();
}

void MoveList::addMove(Move move)
{
	m_moves.push_back(move);
}

void MoveList::sortMoveList()
{
	std::sort(m_moves.begin(), m_moves.end(), move_compare);
}

void MoveList::popMove(std::size_t move_index)
{
	std::swap(m_moves[move_index], m_moves.back());
	m_moves.pop_back();
}

bool MoveList::findMove(const std::size_t source, std::size_t target, Move& move_out) const
{
	for (Move move : m_moves)
	{
		if (move.source() == source)
		{
			if (move.target() == target)
			{
				move_out = move;
				return true;
			}
		}
	}

	return false;
}

bool MoveList::findCastleMove(std::size_t source) const
{
	for (Move move : m_moves)
	{
		if (move.castle())
		{
			if (move.source() == source)
			{
				return true;
			}
		}
	}

	return false;
}

void MoveList::printMoves() const
{
	for (std::size_t i{}; i < m_moves.size(); i++)
	{
		m_moves[i].print();
	}
}

bool MoveList::move_compare(Move a, Move b)
{
	return a.value() < b.value();
}