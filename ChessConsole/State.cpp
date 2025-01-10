#include "State.h"

State::State()
	: m_positions(), m_occupancy(), m_whiteToMove(true), m_enpassantSquare(no_sqr), m_castleRights(0b1111) {}


State::State(const State& state)
	: m_positions(state.m_positions), 
	m_occupancy(state.m_occupancy),
	m_whiteToMove(state.m_whiteToMove), 
	m_enpassantSquare(no_sqr), //always gets reset to no square
	m_castleRights(state.m_castleRights) 
{}

std::uint8_t State::castleRights() const
{
	return m_castleRights;
}

void State::setCastleRights(std::size_t square)
{
	m_castleRights &= castling_rights[square];
}

const std::array<BitBoard, 12>& State::positions() const
{
	return m_positions;
}

const std::array<BitBoard, 3>& State::occupancy() const
{
	return m_occupancy;
}

std::size_t State::enpassantSquare() const
{
	return m_enpassantSquare;
}

Piece State::testPieceType(const std::size_t square) const
{
	for (std::size_t piece{}; piece < PIECE_COUNT; piece++)
	{
		if (m_positions[piece].test(square))
		{
			return static_cast<Piece>(piece);
		}
	}

	return Piece::NO_PIECE;
}

bool State::testCastleRights(const Castle C) const
{
	return static_cast<bool>(m_castleRights & C);
}

void State::setEnpassantSquare(const std::size_t square)
{
	m_enpassantSquare = square;
}

bool State::whiteToMove() const
{
	return m_whiteToMove;
}

void State::flipSide()
{
	m_whiteToMove = !m_whiteToMove;
}

void State::setPiece(const Piece P, const std::size_t square)
{
	m_positions[static_cast<size_t>(P)].set(square);
	m_occupancy[static_cast<size_t>(P / 6)].set(square);
	m_occupancy[Occupancy::BOTH].set(square);
}

void State::popPiece(const Piece P, const std::size_t square)
{
	m_positions[static_cast<size_t>(P)].reset(square);
	m_occupancy[static_cast<size_t>(P / 6)].reset(square);
	m_occupancy[Occupancy::BOTH].reset(square);
}

void State::popSquare(const std::size_t square)
{
	if(m_whiteToMove)
	{
		for (std::size_t piece{ Piece::BPAWN }; piece < PIECE_COUNT; piece++)
		{
			if (m_positions[piece].test(square))
			{
				popPiece(static_cast<Piece>(piece), square);
				break;
			}
		}
	}
	else
	{
		for (std::size_t piece{ Piece::PAWN }; piece < Piece::BPAWN; piece++)
		{
			if (m_positions[piece].test(square))
			{
				popPiece(static_cast<Piece>(piece), square);
				break;
			}
		}
	}
}

void State::moveQuiet(const Piece P, const std::size_t source, const std::size_t target)
{
	popPiece(P, source);
	setPiece(P, target);
}

void State::moveCapture(const Piece P, const std::size_t source, const std::size_t target)
{
	if (m_whiteToMove)
	{
		for (std::size_t piece{ Piece::BPAWN }; piece < PIECE_COUNT; piece++)
		{
			if (m_positions[piece].test(target))
			{
				popPiece(static_cast<Piece>(piece), target);
				break;
			}
		}
	}
	else
	{
		for (std::size_t piece{ Piece::PAWN }; piece < Piece::BPAWN; piece++)
		{
			if (m_positions[piece].test(target))
			{
				popPiece(static_cast<Piece>(piece), target);
				break;
			}
		}
	}

	moveQuiet(P, source, target);
}

void State::printBoard(const bool flipped) const
{
	if (flipped)
	{ 
		for (std::size_t r{}; r < RANK_MAX; r++)
		{
			std::cout << (r + 1) << "   ";
			for (std::size_t f{}; f < FILE_MAX; f++)
			{
				char piece_char = '.';

				for (std::size_t i{}; i < PIECE_COUNT; i++)
				{

					if (m_positions[i].test_rf(7 - r, 7 - f))
					{
						piece_char = piece_to_char[i];
					}
				}

				std::cout << piece_char << " ";
			}
			std::cout << std::endl;
		}

		std::cout << std::endl << "    H G F E D C B A " << (m_whiteToMove ? "white" : "black") << std::endl;
	}
	else
	{
		for (std::size_t r{}; r < RANK_MAX; r++)
		{
			std::cout << (RANK_MAX - r) << "   ";
			for (std::size_t f{}; f < FILE_MAX; f++)
			{
				const std::size_t square{ r * RANK_MAX + f };

				char piece_char = '.';

				for (std::size_t i{}; i < PIECE_COUNT; i++)
				{

					if (m_positions[i].test_rf(r, f))
					{
						piece_char = piece_to_char[i];
					}
				}

				std::cout << piece_char << " ";
			}
			std::cout << std::endl;
		}

		std::cout << std::endl << "    A B C D E F G H " << (m_whiteToMove ? "white" : "black") << std::endl;
	}
}

State State::parse_fen(const std::string_view fen)
{
	State state;

	const std::array<std::string, RANK_MAX> ranks{ split_fen(fen) };

	std::size_t index{};

	for (const auto& rank : ranks)
	{
		for (auto& c : rank)
		{
			if (std::isdigit(c))
			{
				std::size_t digit = static_cast<std::size_t>(c - '0');
				index += digit;
			}
			else
			{
				const std::size_t char_index{ char_to_piece[c] };
				const Piece piece{ static_cast<Piece>(char_index) };

				state.setPiece(piece, index);
				index++;
			}
		}
	}

	return state;
}

std::array<std::string, RANK_MAX> State::split_fen(std::string_view fen)
{
	std::array<std::string, RANK_MAX> split_fen;
	std::size_t index{};

	for (std::size_t i{}; i < RANK_MAX; i++)
	{
		std::string rank;

		while (index < fen.size() && fen[index] != '/')  // Keep adding until '/' or end of string
		{
			rank += fen[index];
			index++;
		}

		split_fen[i] = rank;
		index++;  // Skip the '/' delimiter, but ensure we don't go past the string length
	}

	return split_fen;
}

