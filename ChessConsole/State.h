#pragma once

#include "BitBoard.h"
#include <array>
#include "ChessConstants.hpp"
#include <string>
#include <string_view>
#include "Move.h"

struct State
{
private:

	//class members
	std::array<BitBoard, 12>	m_positions;
	std::array<BitBoard, 3>		m_occupancy;
	std::uint8_t				m_castleRights;
	std::size_t					m_enpassantSquare;
	bool						m_whiteToMove;



	//private methods
	void setPiece(const Piece piece, const std::size_t square);

	void popPiece(const Piece piece, const std::size_t square);

	void moveQuiet(const Piece piece, const std::size_t source, const std::size_t target);

	void moveCapture(const std::size_t source, const std::size_t target, Piece capturedPiece);

	void popSquare(const std::size_t square);



public:

	//constructors
	State();

	State(const State& state);



	//getters
	const std::array<BitBoard, 12>& positions() const;

	const std::array<BitBoard, 3>& occupancy() const;

	std::uint8_t castleRights() const;

	std::size_t enpassantSquare() const;

	bool whiteToMove() const;

	Piece testPieceType(const std::size_t square) const;

	bool testCastleRights(const Castle C) const;



	//modifiers
	void setCastleRights(std::size_t square);

	void setEnpassantSquare(const std::size_t square);

	bool makeMove(const Move move);

	void unmakeMove(const Move move);

	void flipSide();



	//helpers
	void printBoard(const bool flipped, const std::size_t source_square) const;

	static State parse_fen(const std::string_view fen);

	static std::array<std::string, RANK_MAX> split_fen(std::string_view fen);
};