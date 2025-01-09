#pragma once
#include <cstdint>
#include <cstddef>
#include "ChessConstants.hpp"
#include <iostream>

constexpr std::size_t target_shift{ 6 };
constexpr std::size_t promoted_shift{ 12 };
constexpr std::size_t capture_shift{ 13 };
constexpr std::size_t enpassant_shift{ 14 };
constexpr std::size_t castle_shift{ 15 };
constexpr std::size_t piece_shift{ 16 };
constexpr std::size_t double_shift{ 20 };
constexpr std::size_t value_shift{ 21 };

constexpr std::uint32_t source_mask          { 0b00000000000000000000000000111111 };
constexpr std::uint32_t target_mask          { 0b00000000000000000000111111000000 };
constexpr std::uint32_t promoted_mask        { 0b00000000000000000001000000000000 };
constexpr std::uint32_t capture_mask         { 0b00000000000000000010000000000000 };
constexpr std::uint32_t enpassant_mask       { 0b00000000000000000100000000000000 };
constexpr std::uint32_t castle_mask          { 0b00000000000000001000000000000000 };
constexpr std::uint32_t piece_mask           { 0b00000000000011110000000000000000 };
constexpr std::uint32_t double_mask          { 0b00000000000100000000000000000000 };
constexpr std::uint32_t value_mask           { 0b00000000111000000000000000000000 };

struct Move
{
private:
	std::uint32_t m_data;

public:
	//pawn promoted         PIECE FLAG FIRST CAPTURE FLAG SECOND FOR MULTIPLE DEFINITIONS
	Move(const std::size_t source, const std::size_t target, const Piece P, const bool capture);

	//enpassant
	Move(const std::size_t source, const std::size_t target);

	//castle
	Move(const std::size_t target);

	//double pawn push
	Move(const std::size_t source, const std::size_t target, const bool); //arbitrary bool to call constructor
	 
	//other         CAPTURE FLAG FIRST PIECE FLAG SECOND FOR MULTIPLE DEFINITIONS
	Move(const std::size_t source, const std::size_t target, const bool capture, const Piece P, const std::uint32_t value);

	//default
	Move();

	std::size_t source() const;

	std::size_t target() const;

	Move& operator=(const Move& other);

	Piece piece() const;

	bool promoted() const;

	bool capture() const;

	bool enpassant() const;

	bool castle() const;

	bool doublePawnPush() const;

	std::uint32_t value() const;

	void print() const;

	template <Castle C>
	static Move createCastleMove()
	{
		if constexpr (C == Castle::BK)
		{
			return Move(g8);
		}

		if constexpr (C == Castle::BQ)
		{
			return Move(c8);
		}

		if constexpr (C == Castle::WK)
		{
			return Move(g1);
		}

		if constexpr (C == Castle::WQ)
		{
			return Move(c1);
		}
	}
};