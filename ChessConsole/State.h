#pragma once

#include "BitBoard.h"
#include <array>
#include "ChessConstants.hpp"
#include <string>
#include <string_view>
#include <array>
#include "Move.h"

struct State
{
private:
	std::array<BitBoard, 12> m_positions;
	std::array<BitBoard, 3> m_occupancy;

	std::uint8_t m_castleRights;

	std::size_t m_enpassantSquare;

	bool m_whiteToMove;

public:
	State();

	State(const State& state);

	const std::array<BitBoard, 12>& positions() const;

	const std::array<BitBoard, 3>& occupancy() const;

	std::uint8_t castleRights() const;

	void setCastleRights(std::size_t square);

	bool testCastleRights(const Castle C) const;

	std::size_t enpassantSquare() const;

	void setEnpassantSquare(const std::size_t square);

	bool whiteToMove() const;

	void flipSide();

	void printBoard(const bool flipped) const;

	void setPiece(const Piece P, const std::size_t square);

	void popPiece(const Piece P, const std::size_t square);

	void popSquare(const std::size_t square);

	void moveQuiet(const Piece P, const std::size_t source, const std::size_t target);

	void moveCapture(const Piece P, const std::size_t source, const std::size_t target);

	static State parse_fen(const std::string_view fen);

	static std::array<std::string, RANK_MAX> split_fen(std::string_view fen);
};