#include "State.h"

//constructors
State::State()
	: m_positions(), 
	m_occupancy(), 
	m_castleRights(),
	m_enpassantSquare(no_sqr),
	m_whiteToMove(true) {}

State::State(const State& state)
	: m_positions(state.m_positions),
	m_occupancy(state.m_occupancy),
	m_castleRights(state.m_castleRights),
	m_enpassantSquare(no_sqr),
	m_whiteToMove(state.m_whiteToMove) {}



//private methods
void State::setPiece(const Piece piece, const std::size_t square)
{
	m_positions[P].set(square);
	m_occupancy[P / 6].set(square);
	m_occupancy[Occupancy::BOTH].set(square);
}

void State::popPiece(const Piece piece, const std::size_t square)
{
	m_positions[P].reset(square);
	m_occupancy[P / 6].reset(square);
	m_occupancy[Occupancy::BOTH].reset(square);
}

void State::moveQuiet(const Piece piece, const std::size_t source, const std::size_t target)
{
	popPiece(piece, source);
	setPiece(piece, target);
}

void State::moveCapture(const std::size_t source, const std::size_t target, Piece capturedPiece)
{
	popSquare(target);
	moveQuiet(capturedPiece, source, target);
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

bool State::kingInCheck() const
{

}//TODO Impliment

bool State::testCastleRights(const Castle C) const
{
	return static_cast<bool>(m_castleRights & C);
}



//modifiers
void State::setCastleRights(std::size_t square)
{
	m_castleRights &= castling_rights[square];
}

void State::setEnpassantSquare(const std::size_t square)
{
	m_enpassantSquare = square;
}

bool State::makeMove(const Move move)
{
	setEnpassantSquare(no_sqr);

	//unpack
	const std::size_t source = move.source();
	const std::size_t target = move.target();
	const Piece piece = move.piece();
	const bool promoted = move.promoted();
	const bool capture = move.capture();
	const bool double_pawn = move.doublePawnPush();
	const bool enpassant = move.enpassant();
	const bool castle = move.castle();

	//if statements in most efficient order for least number of branching
	if (castle)//TODO: remove moveQuiet and moveCapture they have unnessesary loops and checks. make template function
	{
		if (m_whiteToMove)
		{
			moveQuiet(KING, e1, source);

			if (source == g1)
			{
				moveQuiet(ROOK, h1, f1);
				setCastleRights(h1);
			}
			else
			{
				moveQuiet(ROOK, a1, d1);
				setCastleRights(a1);
			}
		}
		else
		{
			moveQuiet(BKING, e8, source);

			if (source == g8)
			{
				moveQuiet(BROOK, h8, f8);
				setCastleRights(h8);
			}
			else
			{
				moveQuiet(BROOK, a8, d8);
				setCastleRights(a8);
			}
		}
	}
	else
	{
		setCastleRights(source);
		setCastleRights(target);

		//captures
		if (capture)
		{
			if (promoted)
			{
				popPiece(m_whiteToMove ? Piece::PAWN : Piece::BPAWN, source);
				popSquare(target);
				setPiece(piece, target);
			}
			else if (enpassant)//TODO: maybe make move enpassant and other compile time known piece movers
			{
				if (m_whiteToMove)
				{
					popPiece(Piece::PAWN, source);
					popPiece(Piece::BPAWN, target + 8);
					setPiece(Piece::PAWN, target);
				}
				else
				{
					popPiece(Piece::BPAWN, source);
					popPiece(Piece::PAWN, target - 8);
					setPiece(Piece::BPAWN, target);
				}
			}
			else
			{
				moveCapture(source, target, piece);
			}
		}
		//quiets
		else
		{
			if (double_pawn)
			{//TODO: we know its a pawn we dont have to loop through pieces
				moveQuiet(m_whiteToMove ? PAWN : BPAWN, source, target);
				setEnpassantSquare(m_whiteToMove ? source - 8 : source + 8);
			}
			else if (promoted)
			{
				popPiece(m_whiteToMove ? Piece::PAWN : Piece::BPAWN, source);
				setPiece(piece, target);
			}
			else
			{
				moveQuiet(piece, source, target);
			}
		}
	}

	if (kingInCheck())
	{
		return false;
	}
	else
	{
		return true;
	}

}

void State::unmakeMove(const Move move)
{

} //TODO: impliment

void State::flipSide()
{
	m_whiteToMove = !m_whiteToMove;
}

void State::popPiece(const Piece P, const std::size_t square)
{
	m_positions[static_cast<size_t>(P)].reset(square);
	m_occupancy[static_cast<size_t>(P / 6)].reset(square);
	m_occupancy[Occupancy::BOTH].reset(square);
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

