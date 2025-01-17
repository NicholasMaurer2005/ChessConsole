#include "State.h"

//constructors
State::State()
	: m_positions(), 
	m_occupancy(), 
	m_castleRights(),
	m_lastCastleRights(),
	m_enpassantSquare(no_sqr),
	m_lastEnpassantSquare(no_sqr),
	m_whiteToMove(true) {}

State::State(const State& state)
	: m_positions(state.m_positions),
	m_occupancy(state.m_occupancy),
	m_castleRights(state.m_castleRights),
	m_lastCastleRights(),
	m_enpassantSquare(no_sqr),
	m_lastEnpassantSquare(no_sqr),
	m_whiteToMove(state.m_whiteToMove) {}



//private methods
void State::setPiece(const Piece piece, const std::size_t square)
{
	m_positions[piece].set(square);
	m_occupancy[piece / 6].set(square);
	m_occupancy[Occupancy::BOTH].set(square);
}

void State::popPiece(const Piece piece, const std::size_t square)
{
	m_positions[piece].reset(square);
	m_occupancy[piece / 6].reset(square);
	m_occupancy[Occupancy::BOTH].reset(square);
}

void State::moveQuiet(const Piece piece, const std::size_t source, const std::size_t target)
{
	popPiece(piece, source);
	setPiece(piece, target);
}

void State::moveCapture(const std::size_t source, const std::size_t target, Piece piece, Piece capturedPiece)
{
	popPiece(capturedPiece, target);
	moveQuiet(piece, source, target);
}

void State::popSquare(const std::size_t square)
{
	if (m_whiteToMove)
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



//getters
const std::array<BitBoard, 12>& State::positions() const
{
	return m_positions;
}

const std::array<BitBoard, 3>& State::occupancy() const
{
	return m_occupancy;
}

std::uint8_t State::castleRights() const
{
	return m_castleRights;
}

std::size_t State::enpassantSquare() const
{
	return m_enpassantSquare;
}

bool State::whiteToMove() const
{
	return m_whiteToMove;
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



//modifiers
void State::setCastleRights(const std::size_t square)
{
	m_lastCastleRights = m_castleRights;
	m_castleRights &= castling_rights[square];
}

void State::setEnpassantSquare(const std::size_t square)
{
	m_lastEnpassantSquare = m_enpassantSquare;
	m_enpassantSquare = square;
}

void State::makeMove(const Move move)
{
	setEnpassantSquare(no_sqr);

	const std::size_t source{ move.source() };

	if (move.castle())
	{
		if (m_whiteToMove)
		{
			moveQuiet(Piece::KING, e1, source);

			if (source == g1)
			{
				moveQuiet(Piece::ROOK, h1, f1);
				setCastleRights(h1);
			}
			else
			{
				moveQuiet(Piece::ROOK, a1, d1);
				setCastleRights(a1);
			}
		}
		else
		{
			moveQuiet(Piece::BKING, e8, source);

			if (source == g8)
			{
				moveQuiet(Piece::BROOK, h8, f8);
				setCastleRights(h8);
			}
			else
			{
				moveQuiet(Piece::BROOK, a8, d8);
				setCastleRights(a8);
			}
		}
	}
	else
	{
		const std::size_t target{ move.target() };
		const Piece capture{ move.capture()};

		setCastleRights(source);
		setCastleRights(target);

		//captures
		if (capture != Piece::NO_PIECE)
		{
			if (move.promoted())
			{
				popPiece(m_whiteToMove ? Piece::PAWN : Piece::BPAWN, source);
				popPiece(capture, target);
				setPiece(move.piece(), target);
			}
			else if (move.enpassant())
			{
				if (m_whiteToMove)
				{
					popPiece(Piece::BPAWN, target + 8);
					moveQuiet(Piece::PAWN, source, target);
				}
				else
				{
					popPiece(Piece::PAWN, target - 8);
					moveQuiet(Piece::BPAWN, source, target);
				}
			}
			else
			{
				moveCapture(source, target, move.piece(), capture);
			}
		}
		//quiets
		else
		{
			if (move.doublePawnPush())
			{
				moveQuiet(m_whiteToMove ? Piece::PAWN : Piece::BPAWN, source, target);
				setEnpassantSquare(m_whiteToMove ? source - 8 : source + 8);
			}
			else if (move.promoted())
			{
				popPiece(m_whiteToMove ? Piece::PAWN : Piece::BPAWN, source);
				setPiece(move.piece(), target);
			}
			else
			{
				moveQuiet(move.piece(), source, target);
			}
		}
	}
}

void State::unmakeMove(const Move move)
{
	m_enpassantSquare = m_lastEnpassantSquare;

	const std::size_t source{ move.source() };

	if (move.castle())
	{
		m_castleRights = m_lastCastleRights;

		if (m_whiteToMove)
		{
			moveQuiet(KING, source, e1);

			if (source == g1)
			{
				moveQuiet(ROOK, f1, h1);
			}
			else
			{
				moveQuiet(ROOK, d1, a1);
			}
		}
		else
		{
			moveQuiet(BKING, source, e8);

			if (source == g8)
			{
				moveQuiet(BROOK, f8, h8);
			}
			else
			{
				moveQuiet(BROOK, d8, a8);
			}
		}
	}
	else
	{
		const std::size_t target{ move.target() };
		const Piece capture{ move.capture() };

		setCastleRights(source);
		setCastleRights(target);

		//captures
		if (capture != Piece::NO_PIECE)
		{
			if (move.promoted())
			{
				setPiece(m_whiteToMove ? Piece::BPAWN : Piece::PAWN, source);
				setPiece(capture, target);
				popPiece(move.piece(), target);
			}
			else if (move.enpassant())
			{
				if (m_whiteToMove)
				{
					setPiece(Piece::PAWN, target - 8);
					moveQuiet(Piece::BPAWN, target, source);
				}
				else
				{
					setPiece(Piece::BPAWN, target + 8);
					moveQuiet(Piece::PAWN, target, source);
				}
			}
			else
			{
				moveQuiet(move.piece(), target, source);
				setPiece(capture, target);
			}
		}

		//quiets
		else
		{
			if (move.doublePawnPush())
			{
				moveQuiet(m_whiteToMove ? BPAWN : PAWN, target, source);
			}
			else if (move.promoted())
			{
				setPiece(m_whiteToMove ? Piece::BPAWN : Piece::PAWN, source);
				popPiece(move.piece(), target);
			}
			else
			{
				moveQuiet(move.piece(), target, source);
			}
		}
	}
}

void State::flipSide()
{
	m_whiteToMove = !m_whiteToMove;
}

//helpers
void State::printBoard(const bool flipped, const std::size_t source_square) const
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

std::array<std::string, RANK_MAX> State::split_fen(const std::string_view fen)
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

